#include "Globals.h"
#include "ResourceAnimation.h"
#include "Application.h"
#include "Config.h"
#include "ModuleFileSystem.h"

#include "Assimp/scene.h"
#include "Assimp/cimport.h"
#include "Assimp/postprocess.h"

#include "utils/SimpleBinStream.h"

#include "mmgr/mmgr.h"

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

        simple::mem_istream<std::true_type> read_stream(buffer, size);

        uint node_size = 0;
        uint num_channels = 0;

        read_stream >> duration;
        read_stream >> num_channels;

        channels.resize(num_channels);

        for(Channel& channel : channels)
        {
            std::string name;
            read_stream >> name;

            uint num_positions, num_rotations;
            read_stream >> num_positions;
            read_stream >> num_rotations;

            channel.name = HashString(name.c_str());

            channel.positions.resize(num_positions);
            channel.rotations.resize(num_rotations);

            for(uint j=0; j < num_positions; ++j)
            {
                read_stream >> channel.positions[j].x >> channel.positions[j].y >> channel.positions[j].z; 
            }

            for(uint j=0; j < num_rotations; ++j)
            {
                read_stream >> channel.rotations[j].x >> channel.rotations[j].y >> channel.rotations[j].z >> channel.rotations[j].w;  
            }
        }

        uint num_morph_channels;
        read_stream >> num_morph_channels;

        morph_channels.resize(num_morph_channels);

        for(MorphChannel& morph_channel : morph_channels)
        {
            std::string name;
            read_stream >> name;

            morph_channel.name = HashString(name.c_str());

            uint num_keys = 0;
            read_stream >> num_keys;

            morph_channel.keys.resize(num_keys);

            for(WeightList& key : morph_channel.keys)
            {
                uint num_weights = 0;
                read_stream >> num_weights;

                key.resize(num_weights);

                for(float& weight : key)
                {
                    read_stream >> weight;
                }
            }
        }

        return true;
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

        return App->fs->Save(full_path, &data[0], data.size()) > 0;
    }

	std::string output;

	if (App->fs->SaveUnique(output, &data[0], data.size(), LIBRARY_ANIMATION_FOLDER, "anim", "eduanim"))
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

	return App->fs->SaveUnique(output, &data[0], data.size(), LIBRARY_ANIMATION_FOLDER, "anim", "eduanim");
}

// ---------------------------------------------------------
void ResourceAnimation::SaveToStream(simple::mem_ostream<std::true_type>& write_stream) const
{
    write_stream << duration;
    write_stream << channels.size();

    for(const Channel& channel : channels)
    {
        write_stream << channel.name.C_str();
        write_stream << channel.positions.size();
        write_stream << channel.rotations.size();

        for(const float3& position : channel.positions)
        {
            write_stream << position.x << position.y << position.z; 
        }

        for(const Quat& rotation : channel.rotations)
        {
            write_stream << rotation.x << rotation.y << rotation.z << rotation.w;  
        }
    }

    write_stream << morph_channels.size();

    for(const MorphChannel& morph_channel : morph_channels)
    {
        write_stream << morph_channel.name.C_str();

        write_stream << morph_channel.keys.size();

        for(const WeightList& key : morph_channel.keys)
        {
            write_stream << key.size();

            for(float weight : key)
            {
                write_stream << weight;
            }
        }
    }
}

// ---------------------------------------------------------
bool ResourceAnimation::Import(const char* full_path, unsigned first, unsigned last, std::string& output)
{
	aiString assimp_path(".");
	assimp_path.Append(full_path);

	const aiScene* scene = aiImportFile(assimp_path.data, 0);

	if (scene)
	{
        assert(scene->mNumAnimations == 1);

        const aiAnimation* animation = scene->mAnimations[0];
        ResourceAnimation res(0);

		uint duration    = min(last - first, uint(animation->mDuration));
        res.duration     = unsigned(1000*duration/animation->mTicksPerSecond);

        res.channels.resize(animation->mNumChannels);

        for(unsigned i=0; i < animation->mNumChannels; ++i)
        {
            const aiNodeAnim* node = animation->mChannels[i];
            Channel& channel       = res.channels[i];

            channel.name           = HashString(node->mNodeName.C_Str());

            uint pos_first = 0;
            uint pos_last  = 1;

            uint rot_first = 0;
            uint rot_last  = 1;

            if(node->mNumPositionKeys > 1)
            {
                pos_first = min(first, node->mNumPositionKeys);
                pos_last  = max(pos_first, min(last, node->mNumPositionKeys));
            }

            if(node->mNumRotationKeys > 1)
            {
                rot_first = min(first, node->mNumRotationKeys);
                rot_last  = max(rot_first, min(last, node->mNumRotationKeys));
            }

            uint num_positions  = pos_last-pos_first;
            uint num_rotations  = rot_last-rot_first;

            channel.positions.resize(num_positions);
            channel.rotations.resize(num_rotations);

            for(unsigned j=0; j < (pos_last-pos_first); ++j)
            {
                channel.positions[j] = *reinterpret_cast<math::float3*>(&node->mPositionKeys[j+pos_first].mValue);
            }

            for(unsigned j=0; j < (rot_last-rot_first); ++j)
            {
                const aiQuaternion& quat = node->mRotationKeys[j+rot_first].mValue;
                channel.rotations[j] = Quat(quat.x, quat.y, quat.z, quat.w);
            }
        }

        res.morph_channels.resize(animation->mNumMorphMeshChannels);

        for(unsigned i=0; i < animation->mNumMorphMeshChannels; ++i)
        {
            const aiMeshMorphAnim* morph_mesh = animation->mMorphMeshChannels[i];
            MorphChannel& morph_channel       = res.morph_channels[i];

            const char* find = strchr(morph_mesh->mName.C_Str(), '*');
            if(find != nullptr)
            {
                morph_channel.name = HashString(morph_mesh->mName.C_Str(), find-morph_mesh->mName.C_Str());
            }
            else 
            {
                morph_channel.name = HashString(morph_mesh->mName.C_Str());
            }

            uint key_first = 0;
            uint key_last = 1;

            if(morph_mesh->mNumKeys > 1)
            {
                key_first = min(first, morph_mesh->mNumKeys);
                key_last  = max(key_first, min(last, morph_mesh->mNumKeys));
            }

            uint num_keys  = key_last-key_first;
            morph_channel.keys.resize(num_keys);

            for(unsigned j=0; j < (key_last-key_first); ++j)
            {
                uint index = j+key_first;
                WeightList& weights = morph_channel.keys[j];
                weights.resize(morph_mesh->mKeys[index].mNumValuesAndWeights);
                
                for(unsigned k=0; k < morph_mesh->mKeys[index].mNumValuesAndWeights; ++k)
                {
                    assert(morph_mesh->mKeys[index].mValues[k] == k);

                    weights[morph_mesh->mKeys[index].mValues[k]] = float(morph_mesh->mKeys[index].mWeights[k]);
                }
            }
        }

        aiReleaseImport(scene);

        return res.Save(output);
    }

    return false;
}

// ---------------------------------------------------------
uint ResourceAnimation::FindChannelIndex (const HashString& name) const
{
    uint index = 0;

    for(; index < channels.size(); ++index)
    {
        if(channels[index].name == name)
        {
            break;
        }
    }

    return index;
}

