#include "Globals.h"
#include "Application.h"
#include "ModuleTextures.h"
#include "Devil/include/il.h"
#include "Devil/include/ilut.h"

#pragma comment( lib, "Devil/libx86/DevIL.lib" )
#pragma comment( lib, "Devil/libx86/ILUT.lib" )

using namespace std;

ModuleTextures::ModuleTextures(bool start_enabled) : Module(start_enabled)
{
}

// Destructor
ModuleTextures::~ModuleTextures()
{
}

// Called before render is available
bool ModuleTextures::Init()
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
					  
//	for(list<SDL_Texture*>::iterator it = textures.begin(); it != textures.end(); ++it)
//		SDL_DestroyTexture(*it);

//	textures.clear();
	return true;
}

// Load new texture from file path
uint ModuleTextures::Load(const char* file, const char* path)
{
	std::string sPath(path);
	std::string sFile(file);

	uint texId = ilutGLLoadImage((char*) (sPath + sFile).c_str());

	if (texId > 0)
		textures[sFile] = texId;
	else
		LOG("Cannot load texture %s from path %s", file, path);

	return (texId == 0) ? INVALID_TEXTURE : texId;
}

// return a texture id based on the file name
uint ModuleTextures::GetId(const char* path)
{
	uint ret = INVALID_TEXTURE;

	std::map<std::string, uint>::const_iterator it = textures.find(path);

	if (it != textures.end())
		ret = it->second;

	return ret;
}