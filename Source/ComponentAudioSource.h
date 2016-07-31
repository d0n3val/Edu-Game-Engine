#ifndef __COMPONENT_AUDIOSOURCE_H__
#define __COMPONENT_AUDIOSOURCE_H__

// Component to hold an audio source in 3D space

#include "Globals.h"
#include "Component.h"

class ComponentAudioSource : public Component
{
public:
	ComponentAudioSource (GameObject* container);
	~ComponentAudioSource ();

	void OnActivate() override;
	void OnDeActivate() override;

	void OnStart() override;
	void OnUpdate() override;
	void OnFinish() override;

public:
	uint fx_id = 0;
	bool is_2d = false;
	float min_distance = 0.f;
	float max_distance = 0.f;
	int cone_angle_in = 360;
	int cone_angle_out = 360;
	float out_cone_vol = 0.f;
};

#endif // __COMPONENT_AUDIOSOURCE_H__