#ifndef __MODULEEDITOR_H__
#define __MODULEEDITOR_H__

#include "Module.h"
#include <vector>

union SDL_Event;

class Panel;
class PanelConsole;
class PanelGOTree;
class PanelProperties;
class PanelConfiguration;
class PanelAbout;

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
	bool UsingInput() const;
	void Log(const char* entry);
	void LogInputEvent(uint key, uint state);
	void LogFPS(float fps, float ms);

public:
	PanelConsole* console = nullptr;
	PanelGOTree* tree = nullptr;
	PanelProperties* props = nullptr;
	PanelAbout* about = nullptr;
	PanelConfiguration* conf = nullptr;

private:
	std::vector<Panel *> panels;
};

#endif // __MODULEEDITOR_H__