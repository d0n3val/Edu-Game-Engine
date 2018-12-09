#include "Globals.h"
#include "Application.h"
#include "ModuleEditor.h"
#include "ModuleWindow.h"
#include "ModuleFileSystem.h"
#include "ModuleLevelManager.h"
#include "ModuleEditorCamera.h"
#include "ModuleSceneLoader.h"
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
#include <string.h>
#include <algorithm>

using namespace std;

#define IMGUI_IMPL_OPENGL_LOADER_GLEW
#include "imgui.h"
#include "examples/imgui_impl_sdl.h"
#include "examples/imgui_impl_opengl3.h"


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
	
	panels.push_back(console = new PanelConsole());
	panels.push_back(tree = new PanelGOTree());
	panels.push_back(props = new PanelProperties());
	panels.push_back(conf = new PanelConfiguration());
	panels.push_back(about = new PanelAbout());
	panels.push_back(res = new PanelResources());


	return true;
}

bool ModuleEditor::Start(Config * config)
{
    conf->active = config->GetBool("ConfActive", true);
    props->active = config->GetBool("PropsActive", true);

	OnResize(App->window->GetWidth(), App->window->GetHeight());

	return true;
}

void ModuleEditor::Save(Config* config) const 
{
    config->AddBool("ConfActive", conf->active);
    config->AddBool("PropsActive", props->active);
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

			if (ImGui::BeginMenu("View"))
			{
				bool refresh = false;
				refresh = ImGui::MenuItem("Console", nullptr, &console->active) || refresh;
				refresh = ImGui::MenuItem("Scene Hierarchy", nullptr, &tree->active) || refresh;
				refresh = ImGui::MenuItem("Properties", nullptr, &props->active) || refresh;
				refresh = ImGui::MenuItem("Configuration", nullptr, &conf->active) || refresh;
				refresh = ImGui::MenuItem("Resource Browser", nullptr, &res->active) || refresh;

                if(refresh)
                {
                    OnResize(App->window->GetWidth(), App->window->GetHeight());
                }

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

				if (ImGui::MenuItem("About"))
					about->SwitchActive();

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	// Draw all active panels
	for (vector<Panel*>::iterator it = panels.begin(); it != panels.end(); ++it)
	{
		Panel* panel = (*it);

		if (App->input->GetKey(panel->GetShortCut()) == KEY_DOWN)
			panel->SwitchActive();

		if (panel->IsActive())
		{
			ImGui::SetNextWindowPos(ImVec2((float)panel->posx, (float)panel->posy), ImGuiSetCond_Always);
			ImGui::SetNextWindowSize(ImVec2((float)panel->width, (float)panel->height), ImGuiSetCond_Always);
			panel->Draw();
		}
	}

	if (file_dialog == opened)
		LoadFile((file_dialog_filter.length() > 0) ? file_dialog_filter.c_str() : nullptr);
	else
		in_modal = false;

	// Show showcase ? 
	if (showcase)
	{
		ImGui::ShowTestWindow();
		ImGui::ShowMetricsWindow();
	}

	return ret;
}

// Called before quitting
bool ModuleEditor::CleanUp()
{
	LOG("Freeing editor gui");
					  
	for (vector<Panel*>::iterator it = panels.begin(); it != panels.end(); ++it)
		RELEASE(*it);

	panels.clear();

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
	console->width = width - tree->width - conf->width;
	console->posy = height - console->height;

	tree->height = height / 2;
	res->posy = height / 2 + tree->posy;
	res->height = height / 2 - tree->posy;

    if(props->active)
    {
        props->posx = width - props->width;
        props->height = conf->active ? height - props->posy - conf->default_height : height - props->posy;
    }

    if(conf->active)
    {
        conf->posy = props->active ? props->posy + props->height : 0;
        conf->posx = width - conf->width;
        conf->height = props->active ? height - props->height : height;
    }
}

void ModuleEditor::HandleInput(SDL_Event* event)
{
    ImGui_ImplSDL2_ProcessEvent(event);
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

        ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
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
