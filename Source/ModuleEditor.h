#ifndef __MODULEEDITOR_H__
#define __MODULEEDITOR_H__

#include "Module.h"
#include <vector>

union SDL_Event;

class Panel;
class PanelConsole;
class PanelGOTree;
class PanelProperties;

class ModuleEditor : public Module
{
public:
	ModuleEditor(bool start_enabled = true);
	~ModuleEditor();

	bool Init(Config* config = nullptr) override;
	update_status PreUpdate(float dt) override;
	update_status Update(float dt) override;
	bool CleanUp() override;

	void HandleInput(SDL_Event* event);

	void Draw();
	bool IsHoveringGui();
	void Log(const char* entry);

private:
	std::vector<Panel *> panels;

	PanelConsole* console = nullptr;
	PanelGOTree* tree = nullptr;
	PanelProperties* props = nullptr;
};

#endif // __MODULEEDITOR_H__