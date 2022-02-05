#include "Globals.h"
#include "ComponentAudioListener.h"
#include "Application.h"
#include "ModuleAudio.h"

#include "Leaks.h"

// ---------------------------------------------------------
ComponentAudioListener::ComponentAudioListener(GameObject* container) : Component(container, Types::AudioListener)
{
	App->audio->listeners.push_back(this);
}

// ---------------------------------------------------------
ComponentAudioListener::~ComponentAudioListener()
{
	App->audio->listeners.erase(std::remove(App->audio->listeners.begin(), App->audio->listeners.end(),this), App->audio->listeners.end());
}

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
