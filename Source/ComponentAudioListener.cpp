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
void ComponentAudioListener::OnSave(Config * config) const
{
}

// ---------------------------------------------------------
void ComponentAudioListener::OnLoad(Config * config)
{
}