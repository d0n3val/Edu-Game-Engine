#ifndef __MODULEEDITOR_H__
#define __MODULEEDITOR_H__

#include "Module.h"
#include <vector>

#define FILE_MAX 250

union SDL_Event;

class Panel;
class PanelConsole;
class PanelGOTree;
class PanelProperties;
class PanelConfiguration;
class PanelAbout;
class GameObject;

class ModuleEditor : public Module
{
public:
	ModuleEditor(bool start_enabled = true);
	~ModuleEditor();

	bool Init(Config* config = nullptr) override;
	bool Start(Config* config = nullptr) override;
	update_status PreUpdate(float dt) override;
	update_status Update(float dt) override;
	bool CleanUp() override;
	void ReceiveEvent(EventType type, void* userdata) override;

	void OnResize(int width, int height);

	void HandleInput(SDL_Event* event);

	bool FileDialog(const char* extension = nullptr);
	const char* CloseFileDialog();

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
	GameObject* selected = nullptr;

private:
	void LoadFile(const char* filter_extension = nullptr);
	void DrawDirectoryRecursive(const char* directory, const char* filter_extension) ;
	//void SaveFile();

private:
	std::vector<Panel *> panels;
	enum
	{
		closed,
		opened,
		ready_to_close
	} file_dialog = closed;
	std::string file_dialog_filter;

	bool in_modal = false;
	char selected_file[FILE_MAX];
};

#endif // __MODULEEDITOR_H__