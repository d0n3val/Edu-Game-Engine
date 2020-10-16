#include "ResourceAudio.h"
#include "Application.h"
#include "ModuleAudio.h"
#include "Config.h"

#include "Leaks.h"

// ---------------------------------------------------------
ResourceAudio::ResourceAudio(UID uid) : Resource(uid, Resource::Type::audio)
{}

// ---------------------------------------------------------
ResourceAudio::~ResourceAudio()
{
}

// ---------------------------------------------------------
ResourceAudio::Format ResourceAudio::GetFormat() const
{
	return format;
}

// ---------------------------------------------------------
const char * ResourceAudio::GetFormatStr() const
{
	const char* formats[] = { "Stream", "Sample", "Unknown" };
	return formats[format];
}

// ---------------------------------------------------------
bool ResourceAudio::LoadInMemory()
{
	return App->audio->Load(this);
}

// ---------------------------------------------------------
void ResourceAudio::ReleaseFromMemory ()
{
    // \todo:
}

// ---------------------------------------------------------
void ResourceAudio::Save(Config & config) const
{
	Resource::Save(config);
	config.AddInt("Format", format);
}

// ---------------------------------------------------------
void ResourceAudio::Load(const Config & config)
{
	Resource::Load(config);
	format = (Format) config.GetInt("Format", unknown);
}
