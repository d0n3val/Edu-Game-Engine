#ifndef __MODULE_AI_H__
#define __MODULE_AI_H__

#include "Globals.h"
#include "Module.h"

class GameObject;

class ModuleAI : public Module
{
public:

	ModuleAI(bool start_enabled = true);
	~ModuleAI();

	bool Init(Config* config = nullptr) override;
	
	bool Start(Config* config = nullptr) override;
	update_status Update(float dt) override;
	bool CleanUp() override;

	void Save(Config* config) const override;
	void Load(Config* config) override;

private:

private:
};

#endif // __MODULE_AI_H__