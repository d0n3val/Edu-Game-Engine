#include "ResourceTexture.h"

// ---------------------------------------------------------
ResourceTexture::ResourceTexture(UID uid) : Resource(uid, Resource::Type::texture)
{}

// ---------------------------------------------------------
ResourceTexture::~ResourceTexture()
{}

// ---------------------------------------------------------
const char * ResourceTexture::GetFormatStr() const
{
	static const char* formats[] = { 
		"color index", "rgb", "rgba", "bgr", "bgra", "luminance", "unknown" };

	return formats[format];
}

