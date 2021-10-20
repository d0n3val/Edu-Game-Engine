#ifndef __RESOURCE_ANIMATION_H__
#define __RESOURCE_ANIMATION_H__

#include "Resource.h"
#include "Math.h"
#include "HashString.h"
#include "utils/SimpleBinStream.h"

#include <vector>
#include <unordered_map>

class ResourceAnimation : public Resource
{
public:

    struct Channel
    {
        Channel() = default;
        Channel(const Channel& o) = default;
        Channel(Channel&& o) = default;
        Channel& operator=(const Channel& o) = default;
        Channel& operator=(Channel&& o) = default;

        std::unique_ptr<float3[]> positions;
        std::unique_ptr<Quat[]>   rotations;
        uint                      num_positions = 0;
        uint                      num_rotations = 0;
    };

    struct ValueWeights
    {
        uint                                      count = 0;
        std::unique_ptr<std::pair<uint, float>[]> valueWeights;
    };

    struct MorphChannel
    {
        MorphChannel() = default;
        MorphChannel(const MorphChannel& o) = default;
        MorphChannel(MorphChannel&& o) = default;
        MorphChannel& operator=(const MorphChannel& o) = default;
        MorphChannel& operator=(MorphChannel&& o) = default;

        typedef std::unique_ptr<float[]> WeightList;

        
        uint                            numKeys = 0;
        std::unique_ptr<ValueWeights[]> weights;
    };


public:

	ResourceAnimation(UID id);
	virtual ~ResourceAnimation();

	bool            LoadInMemory        () override;
    void            ReleaseFromMemory   () override;

	bool			Save			    () ;
    bool            Save                (std::string& output) const;
	static bool     Import              (const char* full_path, unsigned first, unsigned last, float scale, std::string& output);

	uint            GetDuration         () const { return duration; }

    // channels 

	uint                GetNumChannels      () const { return uint(channels.size()); }
    const Channel*      GetChannel          (const std::string& name) const;
    uint                GetNumMorphChannels () const { return uint(morph_channels.size()); }
    const MorphChannel* GetMorphChannel     (const std::string& name) const; 

private:

    void            SaveToStream        (simple::mem_ostream<std::true_type>& write_stream) const;

    typedef std::unordered_map<std::string, Channel> ChannelList;
    typedef std::unordered_map<std::string, MorphChannel> MorphChannelList;

    uint             duration = 0;
    ChannelList      channels;
    MorphChannelList morph_channels;
};

#endif // __RESOURCE_ANIMATION_H__
