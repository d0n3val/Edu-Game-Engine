#ifndef __MODULEFILESYSTEM_H__
#define __MODULEFILESYSTEM_H__

#include "Module.h"
#include <vector>

struct SDL_RWops;
int close_sdl_rwops(SDL_RWops *rw);

struct aiFileIO;
#include "Bass/include/bass.h"
//struct BASS_FILEPROCS;

class ModuleFileSystem : public Module
{
public:

	ModuleFileSystem(const char* game_path = nullptr);

	// Destructor
	~ModuleFileSystem();

	// Called before render is available
	bool Init(Config* config) override;

	// Called before quitting
	bool CleanUp() override;

	// Utility functions
	bool AddPath(const char* path_or_zip);
	bool Exists(const char* file) const;
	bool IsDirectory(const char* file) const;
	void DiscoverFiles(const char* directory, std::vector<std::string>& file_list, std::vector<std::string>& dir_list) const;

	// Open for Read/Write
	unsigned int Load(const char* file, char** buffer) const;
	SDL_RWops* Load(const char* file) const;
	void* BassLoad(const char* file) const;

	// IO interfaces for other libs to handle files via PHYSfs
	aiFileIO* GetAssimpIO();
	BASS_FILEPROCS* GetBassIO();

	unsigned int Save(const char* file, const char* buffer, unsigned int size) const;

	const char* GetBasePath() const;
	const char* GetWritePath() const;
	const char* GetReadPaths() const;

private:

	void CreateAssimpIO();
	void CreateBassIO();

private:

	aiFileIO* AssimpIO = nullptr;
	BASS_FILEPROCS* BassIO = nullptr;
};

#endif // __MODULEFILESYSTEM_H__