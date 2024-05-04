#include "Globals.h"
#include "ResourceAnimation.h"
#include "Application.h"
#include "Config.h"
#include "ModuleFileSystem.h"

#include "Assimp/scene.h"
#include "Assimp/cimport.h"
#include "Assimp/postprocess.h"

#include "gltf.h"

#include "utils/SimpleBinStream.h"

#include "Leaks.h"

// ---------------------------------------------------------
ResourceAnimation::ResourceAnimation(UID uid) : Resource(uid, Resource::Type::animation)
{}

// ---------------------------------------------------------
ResourceAnimation::~ResourceAnimation()
{
}

// ---------------------------------------------------------
bool ResourceAnimation::LoadInMemory()
{
	if (GetExportedFile() != nullptr)
    {
        char* buffer = nullptr;
        uint size = App->fs->Load(LIBRARY_ANIMATION_FOLDER, GetExportedFile(), &buffer);

        if (buffer != nullptr)
        {
            simple::mem_istream<std::true_type> read_stream(buffer, size);

            uint node_size = 0;
            uint num_channels = 0;

            read_stream >> duration;
            read_stream >> num_channels;

            channels.reserve(num_channels);

            for (uint i = 0; i < num_channels; ++i)
            {
                std::string name;
                read_stream >> name;

                Channel& channel = channels[name];

                read_stream >> channel.num_positions;
                read_stream >> channel.num_rotations;

                channel.positions = std::make_unique<float3[]>(channel.num_positions);
                channel.posTime = std::make_unique<float[]>(channel.num_positions);
                channel.rotations = std::make_unique<Quat[]>(channel.num_rotations);
                channel.rotTime = std::make_unique<float[]>(channel.num_rotations);

                for (uint j = 0; j < channel.num_positions; ++j)
                {
                    read_stream >> channel.positions[j].x >> channel.positions[j].y >> channel.positions[j].z >> channel.posTime[j];
                }

                for (uint j = 0; j < channel.num_rotations; ++j)
                {
                    read_stream >> channel.rotations[j].x >> channel.rotations[j].y >> channel.rotations[j].z >> channel.rotations[j].w >> channel.rotTime[j];
                }
            }

            uint num_morph_channels;
            read_stream >> num_morph_channels;

            morph_channels.reserve(num_morph_channels);

            for (uint i = 0; i < num_morph_channels; ++i)
            {
                std::string name;
                read_stream >> name;

                MorphChannel& morph_channel = morph_channels[name];

                read_stream >> morph_channel.numTime;
                read_stream >> morph_channel.num_weights;

                SDL_assert(morph_channel.num_weights % morph_channel.numTime == 0);

                morph_channel.numTargets = morph_channel.num_weights / morph_channel.numTime;

                morph_channel.weightTime = std::make_unique<float[]>(morph_channel.numTime);
                morph_channel.weights = std::make_unique<float[]>(morph_channel.num_weights);

                for (uint j = 0; j < morph_channel.numTime; ++j)
                    read_stream >> morph_channel.weightTime[j];

                for (uint j = 0; j < morph_channel.num_weights; ++j)
                    read_stream >> morph_channel.weights[j];
            }

            return true;
        }
    }

	return false;
}

// ---------------------------------------------------------
void ResourceAnimation::ReleaseFromMemory()
{
    channels.clear();
    morph_channels.clear();

    duration = 0;
}

// ---------------------------------------------------------
bool ResourceAnimation::Save()
{
    simple::mem_ostream<std::true_type> write_stream;

    SaveToStream(write_stream);

    const std::vector<char>& data = write_stream.get_internal_vec();

    if(exported_file.length() > 0)
    {
		char full_path[250];

		sprintf_s(full_path, 250, "%s%s", LIBRARY_ANIMATION_FOLDER, exported_file.c_str());

        return App->fs->Save(full_path, &data[0], uint(data.size())) > 0;
    }

	std::string output;

	if (App->fs->SaveUnique(output, &data[0], uint(data.size()), LIBRARY_ANIMATION_FOLDER, "anim", "eduanim"))
	{
        App->fs->SplitFilePath(output.c_str(), nullptr, &exported_file);

		return true;
    }

	return false;
}

// ---------------------------------------------------------
bool ResourceAnimation::Save(std::string& output) const
{
    simple::mem_ostream<std::true_type> write_stream;

    SaveToStream(write_stream);

    const std::vector<char>& data = write_stream.get_internal_vec();

	return App->fs->SaveUnique(output, &data[0], uint(data.size()), LIBRARY_ANIMATION_FOLDER, "anim", "eduanim");
}

// ---------------------------------------------------------
void ResourceAnimation::SaveToStream(simple::mem_ostream<std::true_type>& write_stream) const
{
    write_stream << duration;
    write_stream << uint(channels.size());

    for(const std::pair<const std::string, Channel>& channel : channels)
    {
        write_stream << channel.first;
        write_stream << channel.second.num_positions;
        write_stream << channel.second.num_rotations;

        for(uint i=0; i< channel.second.num_positions; ++i)
        {
            write_stream << channel.second.positions[i].x << channel.second.positions[i].y << channel.second.positions[i].z << channel.second.posTime[i];
        }

        for(uint i=0; i< channel.second.num_rotations; ++i)
        {
            write_stream << channel.second.rotations[i].x << channel.second.rotations[i].y << channel.second.rotations[i].z << channel.second.rotations[i].w << channel.second.rotTime[i];
        }
    }

    write_stream << uint(morph_channels.size());

    for(const std::pair<const std::string, MorphChannel>& morph_channel : morph_channels)
    {
        write_stream << morph_channel.first; 
        write_stream << morph_channel.second.numTime;
        write_stream << morph_channel.second.num_weights;

        for (uint i = 0; i < morph_channel.second.numTime; ++i)
            write_stream << morph_channel.second.weightTime[i];

        for (uint i = 0; i < morph_channel.second.num_weights; ++i)
            write_stream << morph_channel.second.weights[i];
    }
}

bool ResourceAnimation::ImportGLTF(const char* full_path, unsigned first, unsigned last, float scale, std::vector<std::string>& output)
{
    tinygltf::TinyGLTF gltfContext;
    tinygltf::Model model;
    std::string error, warning;

    const std::string translation("translation");
    const std::string rotation("rotation");
    const std::string weights("weights");

    if (gltfContext.LoadASCIIFromFile(&model, &error, &warning, full_path))
    {
        output.reserve(model.animations.size());

        for (const tinygltf::Animation& animation : model.animations)
        {
            ResourceAnimation res(0);
            res.duration = 0.0f;

            for (const tinygltf::AnimationChannel& animChannel : animation.channels)
            {
                if (animChannel.target_node >= 0)
                {
                    const std::string& channelName = model.nodes[animChannel.target_node].name;

                    // TODO: Crop
                    const tinygltf::AnimationSampler& sampler = animation.samplers[animChannel.sampler];
                    if (animChannel.target_path == translation)
                    {
                        Channel& resChannel = res.channels[channelName];

                        uint numTime = 0;
                        loadAccessor(resChannel.posTime, numTime, model, sampler.input);
                        loadAccessor(resChannel.positions, resChannel.num_positions, model, sampler.output); 
                        SDL_assert(numTime == resChannel.num_positions);
                        
                        if (numTime > 0)
                        {
                            res.duration = std::max(res.duration, resChannel.posTime[numTime - 1]);
                        }

                        for (uint i = 0; i < resChannel.num_positions; ++i)  resChannel.positions[i] *= scale;
                    }
                    else if (animChannel.target_path == rotation)
                    {
                        Channel& resChannel = res.channels[channelName];

                        uint numTime = 0;
                        loadAccessor(resChannel.rotTime, numTime, model, sampler.input);
                        loadAccessor(resChannel.rotations, resChannel.num_rotations, model, sampler.output);
                        SDL_assert(numTime == resChannel.num_rotations);

                        if (numTime > 0)
                        {
                            res.duration = std::max(res.duration, resChannel.rotTime[numTime - 1]);
                        }
                    }
                    else if (animChannel.target_path == weights)
                    {
                        MorphChannel& resChannel = res.morph_channels[channelName];
                        loadAccessor(resChannel.weightTime, resChannel.numTime, model, sampler.input);
                        loadAccessor(resChannel.weights, resChannel.num_weights, model, sampler.output);
                        SDL_assert(resChannel.num_weights % resChannel.numTime == 0);

                        resChannel.numTargets = resChannel.num_weights / resChannel.numTime;

                        if (resChannel.numTime > 0)
                        {
                            res.duration = std::max(res.duration, resChannel.weightTime[resChannel.numTime - 1]);
                        }
                    }
                }
            }

            output.push_back(std::string());
            if (!res.Save(output.back()))
                return false;
        }

        return true;
    }

    return false;
}

bool ResourceAnimation::ImportAssimp(const char* full_path, unsigned first, unsigned last, float scale, std::string& output)
{
    aiString assimp_path("./");
    assimp_path.Append(full_path);

    const aiScene* scene = aiImportFile(assimp_path.data, 0);

    if (scene)
    {
        assert(scene->mNumAnimations == 1);

        const aiAnimation* animation = scene->mAnimations[0];
        ResourceAnimation res(0);

        uint duration = std::min(last - first, uint(animation->mDuration));
        res.duration = float(1000 * duration / animation->mTicksPerSecond);

        res.channels.reserve(animation->mNumChannels);

        for (unsigned i = 0; i < animation->mNumChannels; ++i)
        {
            const aiNodeAnim* node = animation->mChannels[i];
            Channel& channel = res.channels[std::string(node->mNodeName.C_Str())];

            uint pos_first = 0;
            uint pos_last = 1;

            uint rot_first = 0;
            uint rot_last = 1;

            if (node->mNumPositionKeys > 1)
            {
                pos_first = std::min(first, node->mNumPositionKeys);
                pos_last = std::max(pos_first, std::min(last, node->mNumPositionKeys));
            }

            if (node->mNumRotationKeys > 1)
            {
                rot_first = std::min(first, node->mNumRotationKeys);
                rot_last = std::max(rot_first, std::min(last, node->mNumRotationKeys));
            }

            channel.num_positions = pos_last - pos_first;
            channel.num_rotations = rot_last - rot_first;

            channel.positions = std::make_unique<float3[]>(channel.num_positions);
            channel.rotations = std::make_unique<Quat[]>(channel.num_rotations);

            for (unsigned j = 0; j < (pos_last - pos_first); ++j)
            {
                channel.positions[j] = (*reinterpret_cast<math::float3*>(&node->mPositionKeys[j + pos_first].mValue)) * scale;
            }

            for (unsigned j = 0; j < (rot_last - rot_first); ++j)
            {
                const aiQuaternion& quat = node->mRotationKeys[j + rot_first].mValue;
                channel.rotations[j] = Quat(quat.x, quat.y, quat.z, quat.w);
            }
        }

        res.morph_channels.reserve(animation->mNumMorphMeshChannels);

        for (unsigned i = 0; i < animation->mNumMorphMeshChannels; ++i)
        {
            const aiMeshMorphAnim* morph_mesh = animation->mMorphMeshChannels[i];

            std::string name;

            const char* find = strchr(morph_mesh->mName.C_Str(), '*');
            if (find != nullptr)
            {
                name = std::string(morph_mesh->mName.C_Str(), find - morph_mesh->mName.C_Str());
            }
            else
            {
                name = std::string(morph_mesh->mName.C_Str());
            }

            MorphChannel& morph_channel = res.morph_channels[name];

            uint key_first = 0;
            uint key_last = 1;

            if (morph_mesh->mNumKeys > 1)
            {
                key_first = std::min(first, morph_mesh->mNumKeys);
                key_last = std::max(key_first, std::min(last, morph_mesh->mNumKeys));
            }

            //morph_channel.numKeys = key_last - key_first;
            //morph_channel.num_weights = 0;

            //if (morph_channel.numKeys > 0)
            {
                // \todo: num_weights should come from max num_weights
#if 0
                morph_channel.weights = std::make_unique<ValueWeights[]>(morph_channel.numKeys);

                for (unsigned j = 0; j < (key_last - key_first); ++j)
                {
                    uint key_index = j + key_first;

                    ValueWeights& valueWeights = morph_channel.weights[j];
                    valueWeights.count = morph_mesh->mKeys[key_index].mNumValuesAndWeights;
                    valueWeights.valueWeights = std::make_unique<std::pair<uint, float>[]>(morph_mesh->mKeys[key_index].mNumValuesAndWeights);

                    for (unsigned k = 0; k < morph_mesh->mKeys[key_index].mNumValuesAndWeights; ++k)
                    {
                        valueWeights.valueWeights[k].first = morph_mesh->mKeys[key_index].mValues[k];
                        valueWeights.valueWeights[k].second = float(morph_mesh->mKeys[key_index].mWeights[k]);
                    }
                }
#endif 
            }
        }

        aiReleaseImport(scene);

        return res.Save(output);
    }

    return false;
}

// ---------------------------------------------------------
bool ResourceAnimation::Import(const char* full_path, unsigned first, unsigned last, float scale, std::vector<std::string>& output)
{
    return ImportGLTF(full_path, first, last, scale, output); // || ImportAssimp(full_path, first, last, scale, output);
}

// ---------------------------------------------------------
const ResourceAnimation::Channel* ResourceAnimation::GetChannel(const std::string& name) const
{
    auto it = channels.find(name);
    return it != channels.end() ? &it->second : nullptr;
}

const ResourceAnimation::MorphChannel* ResourceAnimation::GetMorphChannel(const std::string& name) const
{
    auto it = morph_channels.find(name);
    return it != morph_channels.end() ? &it->second : nullptr;
}
