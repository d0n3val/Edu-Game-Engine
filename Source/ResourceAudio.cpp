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
	config.AddInt("Format", format);
	//Resource::Save(config);
}

// ---------------------------------------------------------
void ResourceAudio::Load(const Config & config)
{
	format = (Format) config.GetInt("Format", unknown);
	//Resource::Load(config);
}
