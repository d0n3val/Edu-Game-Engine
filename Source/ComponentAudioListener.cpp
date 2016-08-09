#include "Globals.h"
#include "ComponentAudioListener.h"

// ---------------------------------------------------------
ComponentAudioListener::ComponentAudioListener(GameObject* container) : Component(container)
{
	type = ComponentTypes::AudioListener;
}

// ---------------------------------------------------------
ComponentAudioListener::~ComponentAudioListener()
{}

// ---------------------------------------------------------
void ComponentAudioListener::OnSave(Config& config) const
{
	config.AddFloat("Distance", distance);
	config.AddFloat("Roll Off", roll_off);
	config.AddFloat("Doppler", doppler);
}

// ---------------------------------------------------------
void ComponentAudioListener::OnLoad(Config * config)
{
}