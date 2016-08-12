#include "Resource.h"

// ---------------------------------------------------------
Resource::Resource(UID uid, Resource::Type type) : uid(uid), type(type)
{}

// ---------------------------------------------------------
Resource::~Resource()
{}
Resource::Type Resource::GetType() const
{
	return type;
}
// ---------------------------------------------------------
const char * Resource::GetTypeStr() const
{
	static const char* types[] = {
		"Texture", "Geometry", "Audio Stream", "Audio Sample", "Scene", "Unknown" };
	return types[type];
}
// ---------------------------------------------------------
UID Resource::GetUID() const
{
	return uid;
}
// ---------------------------------------------------------
const char * Resource::GetFile() const
{
	return file.c_str();
}
// ---------------------------------------------------------
const char * Resource::GetExportedFile() const
{
	return exported_file.c_str();
}

