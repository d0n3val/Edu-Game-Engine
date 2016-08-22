#include "Globals.h"
#include "ComponentAudioListener.h"

// ---------------------------------------------------------
ComponentAudioListener::ComponentAudioListener(GameObject* container) : Component(container, Types::AudioListener)
{}

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
	distance = config->GetFloat("Distance", 100.f);
	roll_off = config->GetFloat("Roll Off", 1.f);
	doppler = config->GetFloat("Doppler", 1.f);
}