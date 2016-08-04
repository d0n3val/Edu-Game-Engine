#ifndef __PANELCONFIGURATION_H__
#define __PANELCONFIGURATION_H__

// Editor Panel for all Module's configuration
#include "Panel.h"
#include "Imgui/imgui.h"

class Module;
class ModuleFileSystem;
class ModuleWindow;
class ModuleInput;
class ModuleAudio;
class ModulePhysics3D;
class ModuleRenderer3D;
class ModuleCamera3D;
class ModuleScene;
class ModuleTextures;
class ModuleMeshes;
class ModuleEditor;

class PanelConfiguration : public Panel
{
public:
	PanelConfiguration();
	virtual ~PanelConfiguration();

	void Draw() override;

	bool InitModuleDraw(Module* module);
	void DrawModuleAudio(ModuleAudio * module);
	void DrawModuleFileSystem(ModuleFileSystem * module);
	void DrawModuleInput(ModuleInput * module);
	void AddInput(const char* entry);

private:
    ImGuiTextBuffer input_buf;
    bool need_scroll = false;

};

#endif// __PANELCONFIGURATION_H__