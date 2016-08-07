#ifndef __MODULE_LEVELMANAGER_H__
#define __MODULE_LEVELMANAGER_H__

#include "Globals.h"
#include "Module.h"

class ModuleLevelManager : public Module
{
public:

	ModuleLevelManager(bool start_enabled = true);
	~ModuleLevelManager();

	bool Init(Config* config = nullptr) override;
	bool Start(Config* config = nullptr) override;
	bool CleanUp() override;

	// Load audio assets
	//bool CreateNewEmpty(const char* name);
	bool Load(const char* file);
	//void UnloadCurrent();
	
	bool Save(const char* file = nullptr);

private:

private:
};

#endif // __MODULE_LEVELMANAGER_H__