#include "Globals.h"
#include "Application.h"
#include "OpenGL.h"
#include "ModuleTextures.h"
#include "ModuleFileSystem.h"
#include "Devil/include/il.h"
#include "Devil/include/ilut.h"
#include "ResourceTexture.h"

#include "mmgr/mmgr.h"
#pragma comment( lib, "Devil/libx86/DevIL.lib" )
#pragma comment( lib, "Devil/libx86/ILU.lib" )
#pragma comment( lib, "Devil/libx86/ILUT.lib" )

using namespace std;

ModuleTextures::ModuleTextures(bool start_enabled) : Module("Textures", start_enabled)
{
	ilInit();
	iluInit();
	ilutInit();
	ilutRenderer(ILUT_OPENGL);
}

// Destructor
ModuleTextures::~ModuleTextures()
{
	ilShutDown();
}

// Called before render is available
bool ModuleTextures::Init(Config* config)
{
	LOG("Init Image library using DevIL lib version %d", IL_VERSION);

	return true;
}

// Called before quitting
bool ModuleTextures::CleanUp()
{
	LOG("Freeing textures and Image library");

	return true;
}

// Import new texture from file path
bool ModuleTextures::Import(const char* file, const char* path, string& output_file)
{
	bool ret = false;

	std::string sPath(path);
	std::string sFile(file);

	char* buffer = nullptr;
	uint size = App->fs->Load((char*) (sPath + sFile).c_str(), &buffer);

	if (buffer)
		ret = Import(buffer, size, output_file);

	RELEASE_ARRAY(buffer);

	if(ret == false)
		LOG("Cannot load texture %s from path %s", file, path);

	return ret;
}

bool ModuleTextures::Import(const void * buffer, uint size, string& output_file)
{
	bool ret = false;

	if (buffer)
	{
		ILuint ImageName;				  
		ilGenImages(1, &ImageName);
		ilBindImage(ImageName);

		if (ilLoadL(IL_TYPE_UNKNOWN, (const void*)buffer, size))
		{
			ilEnable(IL_FILE_OVERWRITE);

		    ILuint   size;
		    ILubyte *data; 
			// To pick a specific DXT compression use 
			ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);
		    size = ilSaveL(IL_DDS, NULL, 0 ); // Get the size of the data buffer
			if(size > 0) 
			{
				data = new ILubyte[size]; // allocate data buffer
				if (ilSaveL(IL_DDS, data, size) > 0) // Save with the ilSaveIL function
					ret = App->fs->SaveUnique(output_file, data, size, LIBRARY_TEXTURES_FOLDER, "texture", "dds");

				RELEASE_ARRAY(data);
			}
			ilDeleteImages(1, &ImageName);
		}
	}

	if (ret == false)
		LOG("Cannot load texture from buffer of size %u", size);

	return ret;
}


// Load new texture from file path
bool ModuleTextures::Load(ResourceTexture* resource)
{
	bool ret = false;

	char* buffer = nullptr;
	uint size = App->fs->Load(LIBRARY_TEXTURES_FOLDER, resource->GetExportedFile(), &buffer);

	if (buffer != nullptr && size > 0)
	{
		ILuint ImageName;
		ilGenImages(1, &ImageName);
		ilBindImage(ImageName);

		if (ilLoadL(IL_DDS, (const void*)buffer, size))
		{
			ILinfo i;
			iluGetImageInfo(&i);
			resource->width = i.Width;
			resource->height = i.Height;
			resource->bpp = (uint)i.Bpp;
			resource->depth = i.Depth;
			resource->bytes = i.SizeOfData;

			switch (i.Format)
			{
			case IL_COLOUR_INDEX:
				resource->format = ResourceTexture::color_index;
				break;
			case IL_RGB:
				resource->format = ResourceTexture::rgb;
				break;
			case IL_RGBA:
				resource->format = ResourceTexture::rgba;
				break;
			case IL_BGR:
				resource->format = ResourceTexture::bgr;
				break;
			case IL_BGRA:
				resource->format = ResourceTexture::bgra;
				break;
			case IL_LUMINANCE:
				resource->format = ResourceTexture::luminance;
				break;
			default:
				resource->format = ResourceTexture::unknown;
				break;
			}

			resource->gpu_id = ilutGLBindTexImage();

            if(resource->has_mips)
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }

            ilDeleteImages(1, &ImageName);

            ret = true;
        }
	}
	else
		LOG("Cannot load texture resource %s", resource->GetFile());

	RELEASE_ARRAY(buffer);

	return ret;
}

//  Create checkerboard texture  
#define CHECKERS_WIDTH 64
#define CHECKERS_HEIGHT 64

bool ModuleTextures::LoadCheckers(ResourceTexture * resource)
{
	resource->file = "*Checkers Texture Preset*";
	resource->exported_file = "*Checkers Texture Preset*";

	// http://www.glprogramming.com/red/chapter09.html

	GLubyte checkImage[CHECKERS_HEIGHT][CHECKERS_WIDTH][4];

	for (int i = 0; i < CHECKERS_HEIGHT; i++) {
		for (int j = 0; j < CHECKERS_WIDTH; j++) {
			int c = ((((i&0x8)==0)^(((j&0x8))==0)))*255;
			checkImage[i][j][0] = (GLubyte) c;
			checkImage[i][j][1] = (GLubyte) c;
			checkImage[i][j][2] = (GLubyte) c;
			checkImage[i][j][3] = (GLubyte) 255;
		}
	}

	uint ImageName = 0;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &ImageName );
	glBindTexture(GL_TEXTURE_2D, ImageName );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECKERS_WIDTH, CHECKERS_HEIGHT, 
	   0, GL_RGBA, GL_UNSIGNED_BYTE, checkImage);

	resource->width = CHECKERS_WIDTH;
	resource->height = CHECKERS_HEIGHT;
	resource->bpp = 1;
	resource->depth = 4;
	resource->has_mips = false;
	resource->bytes = sizeof(GLubyte) * CHECKERS_HEIGHT * CHECKERS_WIDTH * 4;
	resource->format = ResourceTexture::rgba;
	resource->gpu_id = ImageName;

	return true;
}

bool ModuleTextures::LoadFallback(ResourceTexture* resource, const float3& color)
{
	resource->file = "*Fallback Texture Preset*";
	resource->exported_file = "*Fallback Texture Preset*";

	GLubyte fallbackImage[3] = { GLubyte(255*color.x), GLubyte(255*color.y), GLubyte(255*color.z) };

	uint ImageName = 0;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &ImageName );
	glBindTexture(GL_TEXTURE_2D, ImageName );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, fallbackImage);

	resource->width = 1;
	resource->height = 1;
	resource->bpp = 1;
	resource->depth = 3;
	resource->has_mips = false;
	resource->bytes = sizeof(GLubyte) * 3;
	resource->format = ResourceTexture::rgb;
	resource->gpu_id = ImageName;

	return true;
}
