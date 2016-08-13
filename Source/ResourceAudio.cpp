#include "ResourceAudio.h"
#include "Config.h"

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
