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

	float distf; // use meters as default
	float rollf; // How fast sound quietens with distance, 1.0 real worl, max 10.0
	float doppf; // Doppler 1.0 real world, max 10.0
};

#endif // __COMPONENT_AUDIOLISTENER_H__