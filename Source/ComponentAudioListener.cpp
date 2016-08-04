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
void ComponentAudioListener::OnActivate()
{
}

// ---------------------------------------------------------
void ComponentAudioListener::OnDeActivate()
{
}

// ---------------------------------------------------------
void ComponentAudioListener::OnStart()
{
}

// ---------------------------------------------------------
void ComponentAudioListener::OnUpdate()
{
}

// ---------------------------------------------------------
void ComponentAudioListener::OnFinish()
{
}