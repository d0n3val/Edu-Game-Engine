#include "Globals.h"
#include "Application.h"
#include "ModuleTextures.h"
#include "ModuleFileSystem.h"
#include "Devil/include/il.h"
#include "Devil/include/ilut.h"

#pragma comment( lib, "Devil/libx86/DevIL.lib" )
#pragma comment( lib, "Devil/libx86/ILUT.lib" )

using namespace std;

ModuleTextures::ModuleTextures(bool start_enabled) : Module("Textures", start_enabled)
{
}

// Destructor
ModuleTextures::~ModuleTextures()
{
}

// Called before render is available
bool ModuleTextures::Init(Config* config)
{
	LOG("Init Image library using DevIL lib version %d", IL_VERSION);

	ilInit();
	ilutInit();

	ilutRenderer(ILUT_OPENGL);

	return true;
}

// Called before quitting
bool ModuleTextures::CleanUp()
{
	LOG("Freeing textures and Image library");
					  
	return true;
}

// Import new texture from file path
const char* ModuleTextures::Import(const char* file, const char* path)
{
	const char* ret = nullptr;

	std::string sPath(path);
	std::string sFile(file);

	char* buffer = nullptr;
	uint size = App->fs->Load((char*) (sPath + sFile).c_str(), &buffer);

	if (buffer)
		ret = Import(buffer, size);

	RELEASE(buffer);

	if(ret == nullptr)
		LOG("Cannot load texture %s from path %s", file, path);

	return ret;
}

const char* ModuleTextures::Import(const void * buffer, uint size)
{
	static uint id = 0;
	static char name[100];
	bool success = false;

	if (buffer)
	{
		sprintf_s(name, 100, "/Library/Textures/tex_%u.dds", ++id);

		ILuint ImageName;				  
		ilGenImages(1, &ImageName);
		ilBindImage(ImageName);

		if (ilLoadL(IL_TYPE_UNKNOWN, (const void*)buffer, size))
		{
			ilEnable(IL_FILE_OVERWRITE);

		    ILuint   size;
		    ILubyte *data; 
			// To pick a specific DXT compression use ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);
		    size = ilSaveL( IL_DDS, NULL, 0 ); // Get the size of the data buffer
			if(size > 0) 
			{
				data = new ILubyte[size]; // allocate data buffer
				success = ilSaveL(IL_DDS, data, size) > 0; // Save with the ilSaveIL function
				App->fs->Save(name, (const char*)data, size);
				RELEASE(data);
			}
			ilDeleteImages(1, &ImageName);
		}
	}

	if (success == false)
	{
		LOG("Cannot load texture from buffer of size %u", size);
		return nullptr;
	}

	return name;
}

// Load new texture from file path
uint ModuleTextures::Load(const char* file, const char* path)
{
	uint texId = 0;

	if (file != nullptr)
	{
		std::string sPath(path);
		std::string sFile(file);

		char* buffer = nullptr;
		uint size = App->fs->Load((char*)(sPath + sFile).c_str(), &buffer);

		if (buffer)
			texId = Load(buffer, size);

		RELEASE(buffer);
	}

	if(texId == 0)
		LOG("Cannot load texture %s from path %s", file, path);

	return texId;
}

uint ModuleTextures::Load(const void * buffer, uint size)
{
	static uint id = 0;
	uint texId = 0;

	if (buffer)
	{
		ILuint ImageName;				  
		ilGenImages(1, &ImageName);
		ilBindImage(ImageName);

		if (ilLoadL(IL_TYPE_UNKNOWN, (const void*)buffer, size))
		{
			texId = ilutGLBindTexImage();
			ilDeleteImages(1, &ImageName);
		}
	}

	if (texId > 0)
	{
		char name[25];
		sprintf_s(name, 25, "tex_%u", ++id);
		textures[string(name)] = texId;
	}
	else
		LOG("Cannot load texture from buffer of size %u", size);

	return texId;
}

// return a texture id based on the file name
uint ModuleTextures::GetId(const char* path)
{
	uint ret = 0;

	std::map<std::string, uint>::const_iterator it = textures.find(path);

	if (it != textures.end())
		ret = it->second;

	return ret;
}