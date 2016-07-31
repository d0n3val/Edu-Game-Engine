#include "Globals.h"
#include "Application.h"
#include "ModuleAudio.h"
#include "ModuleFileSystem.h"
#include "ModuleInput.h" // TODO: remove this after test
#include "Config.h"
#include "Bass/include/bass.h"

#pragma comment( lib, "Bass/libx86/bass.lib" )

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
		float volume = config->GetFloat("Volume", 1.0f);
		BASS_SetVolume(volume);

		float music_volume = config->GetFloat("Music_Volume", 1.0f);
		BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, (DWORD) (music_volume * 10000.0f));

		float fx_volume = config->GetFloat("Fx_Volume", 1.0f);
		BASS_SetConfig(BASS_CONFIG_GVOL_SAMPLE, (DWORD) (fx_volume * 10000.0f));

		PlayMusic(config->GetString("StartMusic", ""), 10.0f);
		LoadFx("Assets/audio/effects/ding.wav");
	}

	return ret;
}

update_status ModuleAudio::PostUpdate(float dt)
{
	if(App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
		PlayFx(0);

	// Update all 3D values
	BASS_Apply3D();

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleAudio::CleanUp()
{
	LOG("Freeing sound FX, closing Mixer and Audio subsystem");

	if (BASS_ChannelIsActive(music) == TRUE)
		BASS_ChannelStop(music);

	for (vector<unsigned long>::iterator it = fx.begin(); it != fx.end(); ++it)
		BASS_SampleFree(*it);

	fx.clear();

	BASS_Free();
	return true;
}

// Play a music file
bool ModuleAudio::PlayMusic(const char* path, float fade_time)
{
	bool ret = true;

	if (BASS_ChannelIsActive(music) == TRUE)
	{
		if (fade_time <= 0.0f)
			BASS_ChannelStop(music);
		else
			BASS_ChannelSlideAttribute(music, BASS_ATTRIB_VOL, 0.0f, DWORD(fade_time * 1000.0f));
	}

	music = BASS_StreamCreateFileUser( STREAMFILE_BUFFER, BASS_SAMPLE_LOOP | BASS_STREAM_AUTOFREE, App->fs->GetBassIO(), App->fs->BassLoad(path) );
	if (music  == 0)
	{
		LOG("BASS_StreamCreateFile() error: %s", BASS_GetErrorString());
		ret = false;
	}
	else
	{
		if (BASS_ChannelPlay(music , FALSE) == FALSE)
		{
			LOG("BASS_ChannelPlay() error: %s", BASS_GetErrorString());
			ret = false;
		}

		if(ret == true && fade_time > 0.0f)
		{
			BASS_ChannelSetAttribute(music, BASS_ATTRIB_VOL, 0.0f );
			BASS_ChannelSlideAttribute(music, BASS_ATTRIB_VOL, 1.0f, DWORD(fade_time * 1000.0f));
		}
	}

	return ret;

	LOG("Successfully playing %s", path);
	return ret;
}

// Load WAV
unsigned int ModuleAudio::LoadFx(const char* path)
{
	unsigned int ret = 0;

	char* buffer = nullptr;
	uint size = App->fs->Load(path, &buffer);

	if (buffer != nullptr)
	{
		HSAMPLE chunk = BASS_SampleLoad(TRUE, buffer, 0, size, 5, BASS_SAMPLE_OVER_VOL);

		if (chunk == 0)
			LOG("BASS_SampleLoad() file [%s] error: %s", path, BASS_GetErrorString());
		else
		{
			fx.push_back(chunk);
			ret = fx.size() - 1;
		}
	}

	RELEASE(buffer);

	return ret;
}

// Play WAV
bool ModuleAudio::PlayFx(unsigned int id, int repeat)
{
	bool ret = false;

	if(id < fx.size())
	{
		HCHANNEL channel = BASS_SampleGetChannel(fx[id], FALSE);
		if(channel == 0)
			LOG("BASS_SampleGetChannel() with id [%u] error: %s", id, BASS_GetErrorString());
		else
		{
			if (BASS_ChannelPlay(channel, TRUE) == FALSE)
				LOG("BASS_ChannelPlay() with id [%u] error: %s", id, BASS_GetErrorString());
			else
				ret = true;
		}
	}

	return ret;
}
