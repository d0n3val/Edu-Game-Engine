#include "Globals.h"
#include "Application.h"
#include "ModuleEditor.h"
#include "ModuleWindow.h"
#include "ModuleFileSystem.h"
#include "ModuleLevelManager.h"
#include "ModuleEditorCamera.h"
#include "ModuleRenderer3D.h"
#include "ModuleInput.h"
#include "GameObject.h"
#include "DebugDraw.h"
#include "Config.h"
#include "OpenGL.h"
#include "Panel.h"
#include "PanelConsole.h"
#include "PanelGOTree.h"
#include "PanelProperties.h"
#include "PanelConfiguration.h"
#include "PanelAbout.h"
#include "PanelResources.h"
#include "Event.h"

#include "imgui_node_editor.h"

#include <string.h>
#include <algorithm>

using namespace std;

#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui.h"
#include "examples/imgui_impl_sdl.h"
#include "examples/imgui_impl_opengl3.h"

#include "Leaks.h"

namespace ed = ax::NodeEditor;

ModuleEditor::ModuleEditor(bool start_enabled) : Module("Editor", start_enabled)
{
	selected_file[0] = '\0';
}

// Destructor
ModuleEditor::~ModuleEditor()
{
}

// Called before render is available
bool ModuleEditor::Init(Config* config)
{
	LOG("Init editor gui with imgui lib version %s", ImGui::GetVersion());

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = SETTINGS_FOLDER "imgui.ini";
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableSetMousePos;  // Enable Keyboard Controls
	io.WantSetMousePos = true;
    ImGui_ImplSDL2_InitForOpenGL(App->window->GetWindow(), App->renderer3D->context);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Setup style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

	// create all panels
	
    tab_panels[TabPanelBottom].name = "Output";
    tab_panels[TabPanelLeft].name = "Hierarchy";
    tab_panels[TabPanelRight].name = "Inspector";

	tab_panels[TabPanelBottom].panels.push_back(console = new PanelConsole());
	tab_panels[TabPanelLeft].panels.push_back(tree = new PanelGOTree());
	tab_panels[TabPanelRight].panels.push_back(props = new PanelProperties());
	tab_panels[TabPanelRight].panels.push_back(conf = new PanelConfiguration());
	tab_panels[TabPanelRight].panels.push_back(about = new PanelAbout());
	tab_panels[TabPanelLeft].panels.push_back(res = new PanelResources());

	return true;
}

bool ModuleEditor::Start(Config * config)
{
    //conf->active = config->GetBool("ConfActive", true);
    //props->active = config->GetBool("PropsActive", true);

	OnResize(App->window->GetWidth(), App->window->GetHeight());

	return true;
}

void ModuleEditor::Save(Config* config) const 
{
    //config->AddBool("ConfActive", conf->active);
    //config->AddBool("PropsActive", props->active);
}

update_status ModuleEditor::PreUpdate(float dt)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(App->window->GetWindow());
    ImGui::NewFrame();

    // \note: needed for guizmo (maybe ImGui_Impl has a bug)
    int mx, my;
	SDL_GetMouseState(&mx, &my);	 
    ImGui::GetIO().MousePos = ImVec2(float(mx), float(my));

    ImGuiIO& io = ImGui::GetIO();
	capture_keyboard = io.WantCaptureKeyboard;
	capture_mouse = io.WantCaptureMouse;

	return UPDATE_CONTINUE;
}

update_status ModuleEditor::Update(float dt)
{
	update_status ret = UPDATE_CONTINUE;

	static bool showcase = false;
	
	// Main menu GUI
	if (draw_menu == true)
	{
		if (ImGui::BeginMainMenuBar())
		{
			bool selected = false;
			if (ImGui::BeginMenu("File"))
			{
				ImGui::MenuItem("New ...");
				ImGui::MenuItem("Load ...");
				if (ImGui::MenuItem("Save ..."))
					App->level->Save("level.json");

				if (ImGui::MenuItem("Quit", "ESC"))
					ret = UPDATE_STOP;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("Gui Demo"))
					showcase = !showcase;

				if (ImGui::MenuItem("Documentation"))
					App->RequestBrowser("https://github.com/d0n3val/Edu-Game-Engine/wiki");

				if (ImGui::MenuItem("Download latest"))
					App->RequestBrowser("https://github.com/d0n3val/Edu-Game-Engine/releases");

				if (ImGui::MenuItem("Report a bug"))
					App->RequestBrowser("https://github.com/d0n3val/Edu-Game-Engine/issues");

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

    for(uint i=0; i< TabPanelCount; ++i)
    {
        const TabPanel& tab = tab_panels[i];
        ImGui::SetNextWindowPos(ImVec2((float)tab.posx, (float)tab.posy), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2((float)tab.width, (float)tab.height), ImGuiCond_Always);
        if(ImGui::Begin(tab.name, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing))
        {
            if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
            {
                // Draw all active panels
                for (vector<Panel*>::const_iterator it = tab.panels.begin(); it != tab.panels.end(); ++it)
                {
                    Panel* panel = (*it);

                    if (ImGui::BeginTabItem(panel->GetName()))
                    {
                        if (panel->IsActive())
                        {
                            panel->Draw();
                        }

                        ImGui::EndTabItem();
                    }
                }

                ImGui::EndTabBar();
            }
            
            ImGui::End();
        }
    }

    if (file_dialog == opened)
        LoadFile((file_dialog_filter.length() > 0) ? file_dialog_filter.c_str() : nullptr);
    else
        in_modal = false;

    // Show showcase ? 
    if (showcase)
    {
        ImGui::ShowDemoWindow();
        ImGui::ShowMetricsWindow();
    }

    return ret;
}

// Called before quitting
bool ModuleEditor::CleanUp()
{
	LOG("Freeing editor gui");
					  
    for(uint i=0; i< TabPanelCount; ++i)
    {
        for (vector<Panel*>::iterator it = tab_panels[i].panels.begin(); it != tab_panels[i].panels.end(); ++it)
        {
            RELEASE(*it);
        }
        
        tab_panels[i].panels.clear();
    }


	console = nullptr; // fix a but of log comming when we already freed the panel

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

	return true;
}

void ModuleEditor::ReceiveEvent(const Event& event)
{
	switch (event.type)
	{
#ifndef _DEBUG
		case Event::play:
		case Event::unpause:
			draw_menu = false;
			console->active = false;
			tree->active = false;
            props->active = false;
            conf->active = false;
            res->active = false;
		break;
		case Event::stop:
		case Event::pause:
			draw_menu = true;
			console->active = true;
			tree->active = true;
            props->active = true;
            conf->active = true;
            res->active = true;
		break;
#endif
		case Event::gameobject_destroyed:
			if(selection_type == SelectionGameObject)
				selected.go = App->level->Validate(selected.go);
			tree->drag = App->level->Validate(tree->drag);
		break;
		case Event::window_resize:
			OnResize(event.point2d.x, event.point2d.y);
		break;
	}
}

void ModuleEditor::DrawDebug()
{
}

void ModuleEditor::OnResize(int width, int height)
{
    // \todo: Viewport
	tab_panels[TabPanelLeft].posx      = 2;
	tab_panels[TabPanelLeft].posy      = 21;
	tab_panels[TabPanelLeft].width     = 350;
	tab_panels[TabPanelLeft].height    = height- tab_panels[TabPanelLeft].posy;

	tab_panels[TabPanelBottom].posx     = tab_panels[TabPanelLeft].posx + tab_panels[TabPanelLeft].width;
	tab_panels[TabPanelBottom].height   = 225;
	tab_panels[TabPanelBottom].posy     = height - tab_panels[TabPanelBottom].height;
	tab_panels[TabPanelBottom].width    = width - tab_panels[TabPanelLeft].width - tab_panels[TabPanelRight].width;

    tab_panels[TabPanelRight].width  = 350;
    tab_panels[TabPanelRight].posy   = 21;
    tab_panels[TabPanelRight].posx   = width - tab_panels[TabPanelRight].width;
    tab_panels[TabPanelRight].height = height - tab_panels[TabPanelRight].posy;
}

void ModuleEditor::HandleInput(SDL_Event* event)
{
    //if(App->GetState() != Application::play)
    {
        ImGui_ImplSDL2_ProcessEvent(event);
    }
}

bool ModuleEditor::FileDialog(const char * extension, const char* from_folder)
{
	bool ret = true;

	switch (file_dialog)
	{
		case closed:
			selected_file[0] = '\0';
			file_dialog_filter = (extension) ? extension : "";
			file_dialog_origin = (from_folder) ? from_folder : "";
			file_dialog = opened;
		case opened:
			ret = false;
		break;
	}

	return ret;
}

const char * ModuleEditor::CloseFileDialog()
{
	if (file_dialog == ready_to_close)
	{
		file_dialog = closed;
		return selected_file[0] ? selected_file : nullptr;
	}
	return nullptr;
}

void ModuleEditor::Draw()
{
	// Debug Draw on selected GameObject

    ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_MakeCurrent(App->window->GetWindow(), App->renderer3D->context);
}

bool ModuleEditor::UsingMouse() const
{
	return capture_mouse;
	//return in_modal || ImGui::IsMouseHoveringAnyWindow() || (ImGui::IsAnyItemActive() && ImGui::IsMouseDragging());
}

bool ModuleEditor::UsingKeyboard() const
{
	return capture_keyboard;
}

void ModuleEditor::Log(const char * entry)
{
	if(console != nullptr)
		console->AddLog(entry);
}

void ModuleEditor::LogInputEvent(uint key, uint state)
{
	static char entry[512];
	static const char* states[] = { "IDLE", "DOWN", "REPEAT", "UP" };

	if (conf != nullptr)
	{
		if(key < 1000)
			sprintf_s(entry, 512, "Keybr: %02u - %s\n", key, states[state]);
		else
			sprintf_s(entry, 512, "Mouse: %02u - %s\n", key - 1000, states[state]);
		conf->AddInput(entry);
	}
}

void ModuleEditor::LogFPS(float fps, float ms)
{
	if (conf != nullptr)
		conf->AddFPS(fps, ms);
}

void ModuleEditor::SetSelected(GameObject * selected, bool focus)
{
	selection_type = SelectionGameObject;
	this->selected.go = selected;
	if (selected != nullptr && focus == true)
	{
		float radius = selected->global_bbox.MinimalEnclosingSphere().r;
		App->camera->CenterOn(selected->GetGlobalPosition(), std::fmaxf(radius, 5.0f) * 3.0f);
		tree->open_selected = true;
	}
}

void ModuleEditor::LoadFile(const char* filter_extension, const char* from_dir)
{
	ImGui::OpenPopup("Load File");
	if (ImGui::BeginPopupModal("Load File", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		in_modal = true;

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
        ImGui::BeginChild("File Browser", ImVec2(0,300), true);
		DrawDirectoryRecursive(from_dir, filter_extension);
        ImGui::EndChild();
        ImGui::PopStyleVar();

		ImGui::PushItemWidth(250.f);
		if (ImGui::InputText("##file_selector", selected_file, FILE_MAX, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
			file_dialog = ready_to_close;

		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("Ok", ImVec2(50, 20)))
			file_dialog = ready_to_close;
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(50, 20)))
		{
			file_dialog = ready_to_close;
			selected_file[0] = '\0';
		}

		ImGui::EndPopup();
	}
	else
    {
		in_modal = false;
    }
}

void ModuleEditor::DrawDirectoryRecursive(const char* directory, const char* filter_extension) 
{
	vector<string> files;
	vector<string> dirs;

	std::string dir((directory) ? directory : "");
	dir += "/";

	App->fs->DiscoverFiles(dir.c_str(), files, dirs);

	for (vector<string>::const_iterator it = dirs.begin(); it != dirs.end(); ++it)
	{
		if (ImGui::TreeNodeEx((dir + (*it)).c_str(), 0, "%s/", (*it).c_str()))
		{
			DrawDirectoryRecursive((dir + (*it)).c_str(), filter_extension);
			ImGui::TreePop();
		}
	}

	std::sort(files.begin(), files.end());

	for (vector<string>::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		const string& str = *it;

		bool ok = true;

		if(filter_extension && str.substr(str.find_last_of(".") + 1) != filter_extension)
			ok = false;

		if (ok && ImGui::TreeNodeEx(str.c_str(), ImGuiTreeNodeFlags_Leaf))
		{
			if (ImGui::IsItemClicked()) {
				sprintf_s(selected_file, FILE_MAX, "%s%s", dir.c_str(), str.c_str());

				if (ImGui::IsMouseDoubleClicked(0))
					file_dialog = ready_to_close;
			}

			ImGui::TreePop();
		}
	}
}
