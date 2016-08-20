#ifndef __PANELCONFIGURATION_H__
#define __PANELCONFIGURATION_H__

// Editor Panel for all Module's configuration
#include "Panel.h"
#include "Imgui/imgui.h"
#include <vector>

#define FPS_LOG_SIZE 100

class Module;
class ModuleFileSystem;
class ModuleWindow;
class ModuleInput;
class ModuleAudio;
class ModulePhysics3D;
class ModuleRenderer3D;
class ModuleEditorCamera;
class ModuleSceneLoader;
class ModuleTextures;
class ModuleMeshes;
class ModuleHardware;
class ModuleEditor;
class ModuleEditorCamera;

class PanelConfiguration : public Panel
{
public:
	PanelConfiguration();
	virtual ~PanelConfiguration();

	void Draw() override;

	bool InitModuleDraw(Module* module);
	void DrawApplication();
	void DrawModuleHardware(ModuleHardware * module);
	void DrawModuleAudio(ModuleAudio * module);
	void DrawModuleFileSystem(ModuleFileSystem * module);
	void DrawModuleInput(ModuleInput * module);
	void DrawModuleWindow(ModuleWindow * module);
	void DrawModuleRenderer(ModuleRenderer3D * module);
	void DrawModuleTextures(ModuleTextures * module);
	void DrawModuleCamera(ModuleEditorCamera * module);
	void DrawModulePhysics(ModulePhysics3D * module);
	void AddInput(const char* entry);
	void AddFPS(float fps, float ms);

private:
    ImGuiTextBuffer input_buf;
    bool need_scroll = false;
	std::vector<float> fps_log;
	std::vector<float> ms_log;
};

#endif// __PANELCONFIGURATION_H__