#ifndef _ANIM_CONTROLLER_H_
#define _ANIM_CONTROLLER_H_

#include "Math.h"

class AnimController
{
	struct Instance
	{
		UID      clip = 0;
		unsigned time = 0;
		bool     loop = true;

		Instance* next          = nullptr;
		unsigned fade_duration = 0;
		unsigned fade_time     = 0;
	};

    Instance* current = nullptr;

public:

    AnimController();
    ~AnimController();

	void            Update              (unsigned elapsed);

	void            Play                (UID clip, unsigned fade_time);
	void            Stop                ();

	bool            GetTransform        (const char* channel_name, math::float3& position, Quat& rotation) const;

private:
    void            UpdateInstance      (Instance* instance, unsigned elapsed);
    void            ReleaseInstance     (Instance* instance);
    bool            GetTransformInstance(Instance* instance, const char* channel_name, float3& position, Quat& rotation) const;
    float3          Interpolate         (const float3& first, const float3& second, float lambda) const;
	Quat            Interpolate         (const Quat& first, const Quat& second, float lambda) const;
};


#endif /* _ANIM_CONTROLLER_H_ */
