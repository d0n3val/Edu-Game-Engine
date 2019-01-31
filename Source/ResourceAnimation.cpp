#include "ResourceAnimation.h"
#include "Application.h"
#include "Config.h"
#include "ModuleFileSystem.h"

#include "Assimp/include/scene.h"
#include "Assimp/include/cimport.h"
#include "Assimp/include/postprocess.h"

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

        read_stream >> duration;
        read_stream >> num_keys;
        read_stream >> num_channels;

        channels = new Channel[num_channels];

        for(uint i=0; i < num_channels; ++i)
        {
            Channel& channel = channels[i];

            read_stream >> channel.name;

            channel.positions = new float3[num_keys];
            channel.rotations = new Quat[num_keys];

            for(uint j=0; j < num_keys; ++j)
            {
                read_stream >> channel.positions[j].x >> channel.positions[j].y >> channel.positions[j].z; 
                read_stream >> channel.rotations[j].x >> channel.rotations[j].y >> channel.rotations[j].z >> channel.rotations[j].w;  
            }
        }

		return true;
    }

	return false;
}

// ---------------------------------------------------------
void ResourceAnimation::ReleaseFromMemory()
{
    for(uint i=0; i< num_channels; ++i)
    {
        delete [] channels[i].positions;
        delete [] channels[i].rotations;
    }

    delete [] channels;

    num_channels = 0;
    num_keys     = 0;
    duration     = 0;
    channels     = nullptr;
}

// ---------------------------------------------------------
void ResourceAnimation::Save(Config & config) const
{
	Resource::Save(config);
}

// ---------------------------------------------------------
void ResourceAnimation::Load(const Config & config)
{
	Resource::Load(config);
}

// ---------------------------------------------------------
bool ResourceAnimation::Save(std::string& output) const
{
    simple::mem_ostream<std::true_type> write_stream;

    write_stream << duration;
    write_stream << num_keys;
    write_stream << num_channels;

    for(uint i=0; i< num_channels; ++i)
    {
        const Channel& channel = channels[i];

        write_stream << channel.name;

        for(uint j=0; j < num_keys; ++j)
        {
            write_stream << channel.positions[j].x << channel.positions[j].y << channel.positions[j].z; 
            write_stream << channel.rotations[j].x << channel.rotations[j].y << channel.rotations[j].z << channel.rotations[j].w;  
        }
    }

    const std::vector<char>& data = write_stream.get_internal_vec();

	return App->fs->SaveUnique(output, &data[0], data.size(), LIBRARY_ANIMATION_FOLDER, "anim", "eduanim");
}

// ---------------------------------------------------------
bool ResourceAnimation::Import(const char* full_path, unsigned first, unsigned last, std::string& output)
{
	const aiScene* scene = aiImportFile(full_path, aiProcessPreset_TargetRealtime_MaxQuality);

	if (scene)
	{
        assert(scene->mNumAnimations == 1);

        const aiAnimation* animation = scene->mAnimations[0];
        ResourceAnimation res(0);

        first = min(last, res.num_keys);

        res.duration     = unsigned(1000*animation->mDuration/animation->mTicksPerSecond);
        res.num_channels = animation->mNumChannels;
        res.num_keys     = min(last-first+1, unsigned(animation->mDuration*animation->mTicksPerSecond-first));
        res.channels     = new Channel[res.num_channels];


        for(unsigned i=0; i < animation->mNumChannels; ++i)
        {
            const aiNodeAnim* node = animation->mChannels[i];
            Channel& channel = res.channels[i];

            channel.name = node->mNodeName.C_Str();

            assert(node->mNumPositionKeys == res.num_keys);
            assert(node->mNumRotationKeys == res.num_keys);

            channel.positions = new math::float3[node->mNumPositionKeys];
            channel.rotations = new Quat[node->mNumRotationKeys];

            for(unsigned j=0; j < res.num_keys; ++j)
            {
                const aiQuaternion& quat = node->mRotationKeys[j+first].mValue;
                channel.rotations[j] = Quat(quat.x, quat.y, quat.z, quat.w);
                channel.positions[j] = *reinterpret_cast<math::float3*>(&node->mPositionKeys[j+first].mValue);
            }
        }

        aiReleaseImport(scene);

        return res.Save(output);
    }

    return false;
}

