#ifndef __RESOURCE_ANIMATION_H__
#define __RESOURCE_ANIMATION_H__

#include "Resource.h"
#include "Math.h"
#include "HashString.h"
#include "utils/SimpleBinStream.h"

class ResourceAnimation : public Resource
{
public:

	ResourceAnimation(UID id);
	virtual ~ResourceAnimation();

	bool            LoadInMemory     () override;
    void            ReleaseFromMemory() override;

	bool			Save			 () ;
    bool            Save             (std::string& output) const;
	static bool     Import           (const char* full_path, unsigned first, unsigned last, std::string& output);

	uint            GetDuration      () const { return duration; }
	uint            GetNumChannels   () const { return channels.size(); }

    uint            FindChannelIndex (const HashString& name) const;

    uint            GetNumPositions  (uint channel_index) const { return channels[channel_index].positions.size(); }
    const float3&   GetPosition      (uint channel_index, uint pos_index) const { return channels[channel_index].positions[pos_index]; }

    uint            GetNumRotations  (uint channel_index) const { return channels[channel_index].rotations.size(); }
    const Quat&     GetRotation      (uint channel_index, uint pos_index) const { return channels[channel_index].rotations[pos_index]; }

private:

    void             SaveToStream     (simple::mem_ostream<std::true_type>& write_stream) const;

private:

	struct Channel
	{
        Channel() = default;
		Channel(const Channel& o) = default;
        Channel(Channel&& o) = default;
		Channel& operator=(const Channel& o) = default;
		Channel& operator=(Channel&& o) = default;

        HashString  name;
        std::vector<float3> positions; 
        std::vector<Quat>   rotations;
	};

    typedef std::vector<float> WeightList;

    struct MorphChannel
    {
        MorphChannel() = default;
		MorphChannel(const MorphChannel& o) = default;
        MorphChannel(MorphChannel&& o) = default;
		MorphChannel& operator=(const MorphChannel& o) = default;
		MorphChannel& operator=(MorphChannel&& o) = default;

        HashString name;
        std::vector<WeightList> keys;
    };

    uint                      duration = 0;
    std::vector<Channel>      channels;
    std::vector<MorphChannel> morph_channels;
};

#endif // __RESOURCE_ANIMATION_H__
