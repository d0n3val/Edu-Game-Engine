#include "ResourceAudio.h"

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
