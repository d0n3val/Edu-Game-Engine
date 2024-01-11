#include "Globals.h"
#include "Application.h"
#include "ModuleAudio.h"
#include "ModuleFileSystem.h"
#include "ModuleLevelManager.h"
#include "ModuleResources.h"
#include "ResourceAudio.h"
#include "GameObject.h"
#include "ComponentAudioListener.h"
#include "ComponentAudioSource.h"
#include "Config.h"
#include "Bass/include/bass.h"
#include "Bass/include/bassenc.h"
#include "Bass/include/bassenc_ogg.h"

#include "Leaks.h"

#pragma comment( lib, "Bass/lib/bass.lib" )
#pragma comment( lib, "Bass/lib/bassenc.lib" )
#pragma comment( lib, "Bass/lib/bassenc_ogg.lib" )

using namespace std;

static const char* BASS_GetErrorString()
{
	switch (BASS_ErrorGetCode())
	{
	case -1: return "mystery problem";
	case 0: return "all is OK";
	case 1: return "memory error";
	case 2: return "can't open the file";
	case 3: return "can't find a free/valid driver";
	case 4: return "the sample buffer was lost";
	case 5: return "invalid handle";
	case 6: return "unsupported sample format";
	case 7: return "invalid position";
	case 8: return "BASS_Init has not been successfully called";
	case 9: return "BASS_Start has not been successfully called";
	case 10: return "SSL/HTTPS support isn't available";
	case 14: return "already initialized/paused/whatever";
	case 18: return "can't get a free channel";
	case 19: return "an illegal type was specified";
	case 20: return "an illegal parameter was specified";
	case 21: return "no 3D support";
	case 22: return "no EAX support";
	case 23: return "illegal device number";
	case 24: return "not playing";
	case 25: return "illegal sample rate";
	case 27: return "the stream is not a file stream";
	case 29: return "no hardware voices available";
	case 31: return "the MOD music has no sequence data";
	case 32: return "no internet connection could be opened";
	case 33: return "couldn't create the file";
	case 34: return "effects are not available";
	case 37: return "requested data is not available";
	case 38: return "the channel is/isn't a 'decoding channel'";
	case 39: return "a sufficient DirectX version is not installed";
	case 40: return "connection timedout";
	case 41: return "unsupported file format";
	case 42: return "unavailable speaker";
	case 43: return "invalid BASS version (used by add-ons)";
	case 44: return "codec is not available or supported";
	case 45: return "the channel/file has ended";
	case 46: return "the device is busy";
	default: return "unknown error code";
	}	  
}

ModuleAudio::ModuleAudio( bool start_enabled) : Module("Audio", start_enabled)
{}

// Destructor
ModuleAudio::~ModuleAudio()
{}

// Called before render is available
bool ModuleAudio::Init(Config* config)
{
	bool ret = true;
	LOG("Loading Audio Mixer");
	
	if (BASS_Init(-1, 44100, BASS_DEVICE_3D, 0, NULL) != TRUE)
	{
		LOG("BASS_Init() error: %s", BASS_GetErrorString());
		ret = false;
	}
	else
	{
		LOG("Using Bass %s", BASSVERSIONTEXT);

		int a, count = 0;
		BASS_DEVICEINFO info;
		for (a = 0; BASS_GetDeviceInfo(a, &info); a++)
			if (info.flags & BASS_DEVICE_ENABLED) // device is enabled
				LOG("Audio device detected: %s", info.name);
	}

	// Settings
	if (config != nullptr && config->IsValid() == true)
	{
		SetVolume(config->GetFloat("Volume", 1.0f));
		SetMusicVolume(config->GetFloat("Music Volume", 1.0f));
		SetFXVolume(config->GetFloat("Fx Volume", 1.0f));
	}

	return ret;
}

bool ModuleAudio::Start(Config * config)
{
	return true;
}

update_status ModuleAudio::PostUpdate(float dt)
{
	// Update all 3D values
	UpdateAudio();

	BASS_Apply3D();

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleAudio::CleanUp()
{
	LOG("Freeing sound FX, closing Mixer and Audio subsystem");

	BASS_Free();
	return true;
}


void ModuleAudio::Save(Config * config) const
{
	config->AddFloat("Volume", GetVolume());
	config->AddFloat("Music Volume", GetMusicVolume());
	config->AddFloat("FX Volume", GetFXVolume());
}

void ModuleAudio::Load(Config * config)
{
	SetVolume(config->GetFloat("Volume", 1.0f));
	SetMusicVolume(config->GetFloat("Music Volume", 1.0f));
	SetFXVolume(config->GetFloat("Fx Volume", 1.0f));

}

void CALLBACK EncodeNewData(HENCODE encoder, DWORD channel, const void* buffer, DWORD lenght, void* user)
{
	LOG("Received encoding chunk of %u bytes for %s", lenght, (const char*)user);
	App->fs->Save((const char*)user, (const char*) buffer, lenght, true);
}

const char * ModuleAudio::ImportSlow(const char * file)
{
	static uint asset_id = 0;
	static char name[80];

	sprintf_s(name, 80, "stream_%u.ogg", ++asset_id);

	BASS_SetConfig( BASS_CONFIG_BUFFER, 65535);

	ulong id = BASS_StreamCreateFileUser( 
				STREAMFILE_NOBUFFER, 
				BASS_STREAM_DECODE | BASS_ASYNCFILE, 
				App->fs->GetBassIO(), 
				App->fs->BassLoad(file) );

	// Start encode paused
	HENCODE encoder = BASS_Encode_OGG_Start(
		id,
		"-m 80",
		BASS_ENCODE_AUTOFREE | BASS_ENCODE_CAST_NOLIMIT,
		EncodeNewData, 
		(void*) name);
	
	if (encoder == 0)
		LOG("BASS_Encode_OGG_Start() error: %s", BASS_GetErrorString());
	else
	{
		// truncate file if it already exist
		App->fs->Remove(name);

		char* buffer = new char[1024*1024];
		BASS_ChannelPlay(id, 0);

		while (BASS_ChannelIsActive(id) == BASS_ACTIVE_PLAYING)
		{
			uint len = BASS_ChannelGetData(id, buffer, 655350);
			BASS_Encode_Write(id, buffer, len);
			LOG("Got %u", len);
		}

		RELEASE(buffer);
	}

	return nullptr;
}

bool ModuleAudio::Import(const char * full_path, string& output_file)
{
	// Try to load and free immediately to check if the resource is valid
	bool ret = false;
	string extension;

	if (full_path != nullptr)
	{
		App->fs->SplitFilePath(full_path, nullptr, nullptr, &extension);

		if (extension == "ogg")
		{
			// OGG files will be streams
			HSTREAM stream = BASS_StreamCreateFileUser( 
				STREAMFILE_BUFFER, BASS_STREAM_AUTOFREE, 
				App->fs->GetBassIO(), App->fs->BassLoad(full_path) );

			if (stream != 0)
			{
				BASS_StreamFree(stream);
				ret = true;
			}
			else
				LOG("BASS_StreamCreateFile() error: %s", BASS_GetErrorString());
		}
		else if (extension == "wav")
		{
			// WAV for samples
			char* buffer = nullptr;
			uint size = App->fs->Load(full_path, &buffer);

			if (buffer != nullptr)
			{
				HSAMPLE sample = BASS_SampleLoad(TRUE, buffer, 0, size, 5, BASS_SAMPLE_OVER_VOL);

				if (sample != 0)
				{
					BASS_SampleFree(sample);
					ret = true;
				}
				else
					LOG("BASS_SampleLoad() file [%s] error: %s", full_path, BASS_GetErrorString());

				RELEASE(buffer);
			}
		}
	}

	// Just copy the file for now
	// TODO: decode and encode again - check ImportSlow()
	if (ret == true)
	{
		char result[250];
		sprintf_s(result, 250, "%s%s_%llu.%s", LIBRARY_AUDIO_FOLDER, "audio", App->resources->GenerateNewUID(), extension.c_str());
		App->fs->NormalizePath(result);
		if (ret = App->fs->Copy(full_path, result))
			output_file = result;
	}

	return ret;
}

bool ModuleAudio::Load(ResourceAudio * resource)
{
	bool ret = false;
	ulong id = 0;

	if (resource != nullptr && resource->GetExportedFile())
	{
		string extension;
		App->fs->SplitFilePath(resource->GetExportedFile(), nullptr, nullptr, &extension);

		if (extension == "ogg")
		{
			// OGG files will be streams
			id = BASS_StreamCreateFileUser( 
				STREAMFILE_BUFFER, 
				BASS_SAMPLE_LOOP | BASS_STREAM_AUTOFREE, 
				App->fs->GetBassIO(), 
				App->fs->BassLoad(resource->GetExportedFile()) );

			if (id != 0)
				resource->format = ResourceAudio::stream;
			else
				LOG("BASS_StreamCreateFile() error: %s", BASS_GetErrorString());
		}
		else if (extension == "wav")
		{
			// WAV for samples
			char* buffer = nullptr;
			uint size = App->fs->Load(resource->GetExportedFile(), &buffer);

			if (buffer != nullptr)
			{
				HSAMPLE sample = BASS_SampleLoad(TRUE, buffer, 0, size, 5, BASS_SAMPLE_OVER_VOL);

				if (sample == 0)
					LOG("BASS_SampleLoad() file [%s] error: %s", resource->GetExportedFile(), BASS_GetErrorString());
				else
				{
					id = BASS_SampleGetChannel(sample, FALSE);

					if (id != 0)
						resource->format = ResourceAudio::sample;
					else
						LOG("BASS_SampleGetChannel() with id [%ul] error: %s", sample, BASS_GetErrorString());
				}

				// since we are not buffering the file, we can safely remove it
				RELEASE(buffer); 
			}
		}
	}

	if(id != 0)
	{
		resource->audio_id = id;
		ret = true;
	}

	return ret;
}

void ModuleAudio::Unload(ulong id)
{
	if (id != 0)
	{
		BASS_CHANNELINFO info;
		BASS_ChannelGetInfo(id, &info );
		if (info.filename != nullptr)
			BASS_SampleFree(id);
		else
			BASS_StreamFree(id);
	}
}

float ModuleAudio::GetVolume() const
{
	return volume;
}

float ModuleAudio::GetMusicVolume() const
{
	return music_volume;
}

float ModuleAudio::GetFXVolume() const
{
	return fx_volume;
}

void ModuleAudio::SetVolume(float new_volume)
{
	volume = new_volume;
	CAP(volume);
	BASS_SetVolume(volume);
}

void ModuleAudio::SetMusicVolume(float new_music_volume)
{
	music_volume = new_music_volume;
	CAP(music_volume);
	BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, (DWORD) (music_volume * 10000.0f));
}

void ModuleAudio::SetFXVolume(float new_fx_volume)
{
	fx_volume = new_fx_volume;
	CAP(fx_volume);
	BASS_SetConfig(BASS_CONFIG_GVOL_SAMPLE, (DWORD) (fx_volume * 10000.0f));
}

void ModuleAudio::UpdateAudio()	const
{
	RecursiveUpdateAudio(App->level->GetRoot());

	/*
	// While in debug, make the debug camera the listener
	BASS_Set3DPosition(
		(BASS_3DVECTOR*)&App->camera->Position, // position
		nullptr, // speed
		(BASS_3DVECTOR*)&App->camera->Z, // front
		(BASS_3DVECTOR*)&App->camera->Y); // up
	*/
}

void ModuleAudio::RecursiveUpdateAudio(GameObject* go) const
{
	for(ComponentAudioListener* listener : listeners)
	{
		UpdateListener(listener);
	}

	for(ComponentAudioSource* source : sources)
	{
		UpdateSource(source);
	}

	/*
	for (list<Component*>::iterator it = go->components.begin(); it != go->components.end(); ++it)
	{
		if ((*it)->IsActive() == false)
			continue;

		switch((*it)->GetType())
		{
			case Component::Types::AudioListener:
				UpdateListener((ComponentAudioListener*) *it);
			 break;

			case Component::Types::AudioSource:
				UpdateSource((ComponentAudioSource*) *it);
			break;
		}
	}

	// Recursive call to all childs
	for (list<GameObject*>::iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		RecursiveUpdateAudio(*it);
		*/
}

void ModuleAudio::UpdateListener(ComponentAudioListener * listener) const
{
	// Setup 3D factors
	BASS_Set3DFactors(listener->distance, listener->roll_off, listener->doppler);

	// Update position and orientation
	const GameObject* go = listener->GetGameObject();
    float3 pos = go->GetGlobalTransformation().TranslatePart();
    float3 front = go->GetGlobalTransformation().WorldZ();
    float3 up = go->GetGlobalTransformation().WorldY();
	BASS_Set3DPosition(
		(BASS_3DVECTOR*)&pos, // position
		nullptr, // speed
		(BASS_3DVECTOR*)&front, // front
		(BASS_3DVECTOR*)&up); // up
}

void ModuleAudio::UpdateSource(ComponentAudioSource* source) const
{
	if (source == nullptr)
		return;

	const ResourceAudio* resource = (const ResourceAudio*) source->GetResource();

	if (resource == nullptr)
		return;

	ulong id = resource->audio_id;

	if (id == 0)
		return;

	switch (source->current_state)
	{
		case ComponentAudioSource::state::playing:
		{
			// Setup 3D attributes for this gameobject
			BASS_ChannelSet3DAttributes(id,
				source->is_2d ? BASS_3DMODE_OFF : BASS_3DMODE_NORMAL,
				source->min_distance,
				source->max_distance,
				source->cone_angle_in,
				source->cone_angle_out,
				source->out_cone_vol);

			// Update 3D position
			const GameObject* go = source->GetGameObject();
            float3 pos = go->GetGlobalPosition();
            float3 front = go->GetGlobalTransformation().WorldZ();
			BASS_ChannelSet3DPosition(id,
				(BASS_3DVECTOR*)&pos, // position
				(BASS_3DVECTOR*)&front, // front
				nullptr); // velocity
		} break;

		case ComponentAudioSource::state::waiting_to_play:
		{
			if (BASS_ChannelPlay(id, FALSE) == FALSE)
				LOG("BASS_ChannelPlay() with channel [%ul] error: %s", id, BASS_GetErrorString());
			else
			{
				BASS_ChannelSetAttribute(id, BASS_ATTRIB_VOL, 0.0f );
				BASS_ChannelSlideAttribute(id, BASS_ATTRIB_VOL, 1.0f, DWORD(source->fade_in * 1000.0f));
				source->current_state = ComponentAudioSource::state::playing;
			}
		} break;

		case ComponentAudioSource::state::waiting_to_stop:
		{
			if (BASS_ChannelStop(id) == FALSE)
				LOG("BASS_ChannelStop() with channel [%ul] error: %s", id, BASS_GetErrorString());
			else
			{
				// TODO: test
				BASS_ChannelSlideAttribute(id, BASS_ATTRIB_VOL, 0.0f, DWORD(source->fade_out * 1000.0f));
				source->current_state = ComponentAudioSource::state::stopped;
			}
		} break;

		case ComponentAudioSource::state::waiting_to_pause:
		{
			if (BASS_ChannelPause(id) == FALSE)
				LOG("BASS_ChannelPause() with channel [%ul] error: %s", id, BASS_GetErrorString());
			else
				source->current_state = ComponentAudioSource::state::paused;
		} break;

		case ComponentAudioSource::state::waiting_to_unpause:
		{
			if (BASS_ChannelPlay(id, FALSE) == FALSE)
				LOG("BASS_ChannelPlay() with channel [%ul] error: %s", id, BASS_GetErrorString());
			else
				source->current_state = ComponentAudioSource::state::playing;
		} break;
	}
}
