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
        read_stream >> num_channels;

        channels = new Channel[num_channels];

        for(uint i=0; i < num_channels; ++i)
        {
            Channel& channel = channels[i];

            read_stream >> channel.name;
            read_stream >> channel.num_positions;
            read_stream >> channel.num_rotations;

            channel.positions = new float3[channel.num_positions];
            channel.rotations = new Quat[channel.num_rotations];

            for(uint j=0; j < channel.num_positions; ++j)
            {
                read_stream >> channel.positions[j].x >> channel.positions[j].y >> channel.positions[j].z; 
            }

            for(uint j=0; j < channel.num_rotations; ++j)
            {
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
    write_stream << num_channels;

    for(uint i=0; i< num_channels; ++i)
    {
        const Channel& channel = channels[i];

        write_stream << channel.name;
        write_stream << channel.num_positions;
        write_stream << channel.num_rotations;

        for(uint j=0; j < channel.num_positions; ++j)
        {
            write_stream << channel.positions[j].x << channel.positions[j].y << channel.positions[j].z; 
        }

        for(uint j=0; j < channel.num_rotations; ++j)
        {
            write_stream << channel.rotations[j].x << channel.rotations[j].y << channel.rotations[j].z << channel.rotations[j].w;  
        }
    }

    const std::vector<char>& data = write_stream.get_internal_vec();

	return App->fs->SaveUnique(output, &data[0], data.size(), LIBRARY_ANIMATION_FOLDER, "anim", "eduanim");
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

        res.duration     = unsigned(1000*animation->mDuration/animation->mTicksPerSecond);
        res.num_channels = animation->mNumChannels;
        res.channels     = new Channel[res.num_channels];

        for(unsigned i=0; i < animation->mNumChannels; ++i)
        {
            const aiNodeAnim* node = animation->mChannels[i];
            Channel& channel       = res.channels[i];

            channel.name           = node->mNodeName.C_Str();
            channel.num_positions  = node->mNumPositionKeys;
            channel.num_rotations  = node->mNumRotationKeys;

            uint pos_first = min(first, channel.num_positions);
            uint pos_last  = max(first, min(last, channel.num_positions));

            uint rot_first = min(first, channel.num_rotations);
            uint rot_last  = max(first, min(last, channel.num_rotations));

            channel.positions = new math::float3[channel.num_positions];
            channel.rotations = new Quat[channel.num_rotations];

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

        aiReleaseImport(scene);

        return res.Save(output);
    }

    return false;
}

