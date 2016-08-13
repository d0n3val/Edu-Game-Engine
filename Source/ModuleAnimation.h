#ifndef __MODULEANIMATION_H__
#define __MODULEANIMATION_H__

#include "Module.h"

class ModuleAnimation : public Module
{
public:

	ModuleAnimation(bool start_active = true);
	~ModuleAnimation();

	bool Init(Config* config) override;
	bool CleanUp() override;

};

#endif // __MODULEANIMATION_H__