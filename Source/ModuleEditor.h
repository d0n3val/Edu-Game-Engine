#ifndef __MODULEEDITOR_H__
#define __MODULEEDITOR_H__

#include "Module.h"
#include <vector>
#include <variant>
#include <memory>

#define FILE_MAX 250

union SDL_Event;

class Panel;
class PanelConsole;
class PanelGOTree;
class PanelProperties;
class PanelConfiguration;
class PanelAbout;
class PanelResources;
class PanelQuickBar;
class GameObject;
class DirLight;
class AmbientLight;
class PointLight;
class SpotLight;
class QuadLight;
class SphereLight;
class Skybox;

class ModuleEditor : public Module
{
public:

	PanelConsole* console = nullptr;
	PanelGOTree* tree = nullptr;
	PanelProperties* props = nullptr;
	PanelAbout* about = nullptr;
	PanelConfiguration* conf = nullptr;
	PanelResources* res= nullptr;

    enum SelectionType
    {
        SelGameObject = 0,
        SelDirLight,
        SelAmbientLight,
        SelPointLight,
        SelSpotLight,
        SelSkybox
    };

    typedef std::variant<GameObject*, DirLight*, PointLight*, SpotLight*, QuadLight*, SphereLight*, Skybox*> SelectionVariant;


    enum TabPanelEnum
    {
        TabPanelLeft = 0,
        TabPanelRight ,
        TabPanelBottom ,
        TabPanelCount
    };

public:
	ModuleEditor(bool start_enabled = true);
	~ModuleEditor();

	bool Init(Config* config = nullptr) override;
	bool Start(Config* config = nullptr) override;
	update_status PreUpdate(float dt) override;
	update_status Update(float dt) override;
	bool CleanUp() override;
	void ReceiveEvent(const Event& event) override;
	void DrawDebug() override;
	virtual void Save(Config* config) const override;

	// TODO Save/load panel activation

	void OnResize(int width, int height);

	void HandleInput(SDL_Event* event);

	bool FileDialog(const char* extension = nullptr, const char* from_folder = nullptr);
	const char* CloseFileDialog();

	void Draw();
	bool UsingMouse() const;
	bool UsingKeyboard() const;
	void Log(const char* entry);
	void LogInputEvent(uint key, uint state);
	void LogFPS(float fps, float ms);

    int GetWidth(TabPanelEnum panel) const { return tab_panels[panel].width; }
    int GetHeight(TabPanelEnum panel) const { return tab_panels[panel].height; }
    int GetPosX(TabPanelEnum panel) const { return tab_panels[panel].posx; }
    int GetPosY(TabPanelEnum panel) const { return tab_panels[panel].posy; }

    const SelectionVariant& GetSelection() const { return selected; }

    template<typename Arg>
    void SetSelected(Arg && arg) { selected = std::forward<Arg>(arg); }
    void ClearSelected() { selected = {}; }

private:

	void LoadFile(const char* filter_extension = nullptr, const char* from_dir = nullptr);
	void DrawDirectoryRecursive(const char* directory, const char* filter_extension) ;

private:

    struct TabPanel
    {
        int width  = 0;
        int height = 0; 
        int posx   = 0; 
        int posy   = 0;
        char* name = nullptr;

        std::vector<Panel *> panels;
    };

    TabPanel tab_panels[TabPanelCount];

	enum
	{
		closed,
		opened,
		ready_to_close
	} file_dialog = closed;
	std::string file_dialog_filter;
	std::string file_dialog_origin;

	bool capture_mouse = false;
	bool capture_keyboard = false;
	bool in_modal = false;
	char selected_file[FILE_MAX];
	bool draw_menu = true;

    SelectionVariant selected;

};

#endif // __MODULEEDITOR_H__
