#ifndef __COMPONENT_AUDIOLISTENER_H__
#define __COMPONENT_AUDIOLISTENER_H__

// Component to hold an audio listener

#include "Component.h"

class ComponentAudioListener : public Component
{
public:
	ComponentAudioListener (GameObject* container);
	~ComponentAudioListener ();

	void OnActivate() override;
	void OnDeActivate() override;

	void OnStart() override;
	void OnUpdate() override;
	void OnFinish() override;

public:

	float distance = 100.0f; // use meters as default
	float roll_off = 1.0f; // How fast sound quietens with distance, 1.0 real worl, max 10.0
	float doppler = 1.0f; // Doppler 1.0 real world, max 10.0
};

#endif // __COMPONENT_AUDIOLISTENER_H__