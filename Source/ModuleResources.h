#ifndef __MODULERESOURCES_H__
#define __MODULERESOURCES_H__

#include "Module.h"

#define ASSET_FOLDER "/Assets/"

class ModuleResources : public Module
{
public:

	ModuleResources(bool start_enabled = true);

	// Destructor
	~ModuleResources();

	bool Init(Config* config) override;
	bool CleanUp() override;
	void ReceiveEvent(const Event& event) override;

	bool ImportFile(const char* full_path, const char* destination = nullptr);

private:
	std::string asset_folder;
};

#endif // __MODULERESOURCES_H__