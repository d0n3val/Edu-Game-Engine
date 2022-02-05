#include "Globals.h"
#include "ComponentAudioSource.h"
#include "Application.h"
#include "ModuleAudio.h"
#include "ModuleResources.h"
#include "ResourceAudio.h"
#include "GameObject.h"

#include "Leaks.h"

// ---------------------------------------------------------
ComponentAudioSource::ComponentAudioSource(GameObject* container) : Component(container, Types::AudioSource)
{
	App->audio->sources.push_back(this);
}

// ---------------------------------------------------------
ComponentAudioSource::~ComponentAudioSource()
{
	App->audio->sources.erase(std::remove(App->audio->sources.begin(), App->audio->sources.end(),this), App->audio->sources.end());
}

// ---------------------------------------------------------
void ComponentAudioSource::GetBoundingBox(AABB & box) const
{
	box.Enclose(Sphere(float3::zero, max_distance));
}

// ---------------------------------------------------------
void ComponentAudioSource::OnSave(Config& config) const
{
	ComponentWithResource::OnSaveResource(config);

	config.AddBool("Is 2D", is_2d);
	config.AddFloat("Min Distance", min_distance);
	config.AddFloat("Max Distance", max_distance);
	config.AddInt("Cone Angle In", cone_angle_in);
	config.AddInt("Cone Angle Out", cone_angle_out);
	config.AddFloat("Out Cone Vol", out_cone_vol);
	config.AddFloat("Fade In", fade_in);
	config.AddFloat("Fade Out", fade_out);
}

// ---------------------------------------------------------
void ComponentAudioSource::OnLoad(Config * config)
{
	ComponentWithResource::OnLoadResource(config);

	is_2d = config->GetBool("Is 2D", false);
	min_distance = config->GetFloat("Min Distance", 0.f);
	max_distance = config->GetFloat("Max Distance", 0.f);
	cone_angle_in = config->GetInt("Cone Angle In", 360);
	cone_angle_out = config->GetInt("Cone Angle Out", 360);
	out_cone_vol = config->GetFloat("Out Cone Vol", 0.f);
	fade_in = config->GetFloat("Fade In", 1.f);
	fade_out = config->GetFloat("Fade Out", 1.f);
}

// ---------------------------------------------------------
bool ComponentAudioSource::SetResource(UID resource)
{
	bool ret = false;

	if(current_state != state::unloaded)
		Unload();

	if (resource != 0 && resource != this->resource)
	{
		Resource* res = App->resources->Get(resource);
		if (res != nullptr && res->GetType() == Resource::texture)
		{
			if(res->LoadToMemory() == true)
			{
				this->resource = resource;
				game_object->RecalculateBoundingBox();
				current_state = state::stopped;
				ret = true;
			}
		}
	}

	return ret;
}

// ---------------------------------------------------------
void ComponentAudioSource::Unload()
{
	// TODO: still not a formal way to unload resources
	const ResourceAudio* res = (const ResourceAudio*) GetResource();
	if (res != nullptr && res->audio_id != 0)
	{
		App->audio->Unload(res->audio_id);
		current_state = state::unloaded;
	}
}

// ---------------------------------------------------------
void ComponentAudioSource::OnDeActivate()
{
	if (IsPlaying() == true)
		Stop();
}

// ---------------------------------------------------------
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

// ---------------------------------------------------------
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

// ---------------------------------------------------------
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

// ---------------------------------------------------------
void ComponentAudioSource::Stop()
{
	if (current_state == state::playing)
		current_state = state::waiting_to_stop;
}

// ---------------------------------------------------------
bool ComponentAudioSource::IsPlaying() const
{
	return current_state == state::playing;
}

// ---------------------------------------------------------
bool ComponentAudioSource::IsPaused() const
{
	return current_state == state::paused;
}

// ---------------------------------------------------------
int ComponentAudioSource::GetCurrentState() const
{
	return current_state;
}
