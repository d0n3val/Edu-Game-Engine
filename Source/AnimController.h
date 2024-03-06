#ifndef _ANIM_CONTROLLER_H_
#define _ANIM_CONTROLLER_H_

#include "Math.h"
#include "HashString.h"

#include <vector>
#include <span>

class AnimController
{
	struct Instance
	{
		UID      clip  = 0;
		float    time  = 0.0f;
		bool     loop  = true;
        float    speed = 1.0;

		Instance* next         = nullptr;
		unsigned fade_duration = 0;
		unsigned fade_time     = 0;
	};

    Instance* current = nullptr;

    mutable std::vector<float> tmpWeights0, tmpWeights1;
public:

    AnimController();
    ~AnimController();

	void                    Update              (unsigned elapsed);

	void                    Play                (UID clip, bool loop, unsigned fade_time);
	void                    Stop                ();

    float                   GetSpeed            () const { return current ? current->speed : 0.0f; }
    void                    SetSpeed            (float speed) { if(current) current->speed = speed; }

	bool                    GetTransform        (const std::string& channel_name, math::float3& position, Quat& rotation) const;
    std::span<const float>  GetWeights          (const std::string& morph_name) const;

private:
    void            UpdateInstance      (Instance* instance, unsigned elapsed);
    void            ReleaseInstance     (Instance* instance);
    bool            GetTransformInstance(Instance* instance, const std::string& channel_name, float3& position, Quat& rotation) const;
    bool            GetWeightsInstance  (Instance* instance, const std::string& morph_name, float* weights, uint num_weights) const;
};


#endif /* _ANIM_CONTROLLER_H_ */
