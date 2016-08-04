#include "Globals.h"
#include "ComponentAudioSource.h"
#include "Application.h"
#include "ModuleAudio.h"

// ---------------------------------------------------------
ComponentAudioSource::ComponentAudioSource(GameObject* container) : Component(container)
{
	type = ComponentTypes::AudioSource;
}

// ---------------------------------------------------------
ComponentAudioSource::~ComponentAudioSource()
{}

// ---------------------------------------------------------
bool ComponentAudioSource::LoadFile(const char * file)
{
	if(current_state != state::unloaded)
		Unload();

	if (file != nullptr)
		id = App->audio->Load(file);

	if (id != 0)
		current_state = state::stopped;

	return id != 0;
}

// ---------------------------------------------------------
const char * ComponentAudioSource::GetFile() const
{
	return App->audio->GetFile(id);
}

// ---------------------------------------------------------
void ComponentAudioSource::Unload()
{
	if (id != 0)
	{
		App->audio->Unload(id);
		current_state = state::unloaded;
	}
}

bool ComponentAudioSource::Play()
{
	bool ret = false;

	if (current_state == state::stopped)
	{
		current_state = state::waiting_to_play;
		ret = true;
	}

	return ret;
}

bool ComponentAudioSource::Pause()
{
	bool ret = false;

	if (current_state == state::playing)
	{
		current_state = state::waiting_to_pause;
		ret = true;
	}

	return ret;
}

bool ComponentAudioSource::UnPause()
{
	bool ret = false;

	if (current_state == state::paused)
	{
		current_state = state::waiting_to_unpause;
		ret = true;
	}

	return ret;
}

void ComponentAudioSource::Stop()
{
	if (current_state == state::playing)
		current_state = state::waiting_to_stop;
}

bool ComponentAudioSource::IsPlaying() const
{
	return current_state == state::playing;
}

bool ComponentAudioSource::IsPaused() const
{
	return current_state == state::paused;
}

int ComponentAudioSource::GetCurrentState() const
{
	return current_state;
}
