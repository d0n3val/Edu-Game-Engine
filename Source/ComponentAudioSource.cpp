#include "Globals.h"
#include "ComponentAudioSource.h"

// ---------------------------------------------------------
ComponentAudioSource::ComponentAudioSource(GameObject* container) : Component(container)
{
	type = ComponentTypes::AudioSource;
}

// ---------------------------------------------------------
ComponentAudioSource::~ComponentAudioSource()
{}

// ---------------------------------------------------------
void ComponentAudioSource::OnActivate()
{
}

// ---------------------------------------------------------
void ComponentAudioSource::OnDeActivate()
{
}

// ---------------------------------------------------------
void ComponentAudioSource::OnStart()
{
}

// ---------------------------------------------------------
void ComponentAudioSource::OnUpdate()
{
}

// ---------------------------------------------------------
void ComponentAudioSource::OnFinish()
{
}