#include "Globals.h"
#include "Application.h"
#include "ModuleEditor.h"
#include "ModuleWindow.h"
#include "ModuleFileSystem.h"
#include "ModuleLevelManager.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "GameObject.h"
#include "DebugDraw.h"
#include "Config.h"
#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_sdl_gl3.h"
#include "OpenGL.h"
#include "Panel.h"
#include "PanelConsole.h"
#include "PanelGOTree.h"
#include "PanelProperties.h"
#include "PanelConfiguration.h"
#include "PanelAbout.h"
#include <string.h>
#include <algorithm>

using namespace std;

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
	LOG("Init editor gui with imgui lib");

    ImGui_ImplSdlGL3_Init(App->window->GetWindow());

	// create all panels
	panels.push_back(console = new PanelConsole());
	panels.push_back(tree = new PanelGOTree());
	panels.push_back(props = new PanelProperties());
	panels.push_back(conf = new PanelConfiguration());
	panels.push_back(about = new PanelAbout());

	return true;
}

update_status ModuleEditor::PreUpdate(float dt)
{
    ImGui_ImplSdlGL3_NewFrame(App->window->GetWindow());
	return UPDATE_CONTINUE;
}

update_status ModuleEditor::Update(float dt)
{
	update_status ret = UPDATE_CONTINUE;

	static bool showcase = false;
	
	// Main menu GUI
	if (ImGui::BeginMainMenuBar())
	{
		bool selected = false;
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("New ...");
			if (ImGui::MenuItem("Load ..."))
				load_file_active = true;
				
			if (ImGui::MenuItem("Save ..."))
				App->level->Save(nullptr);

			if (ImGui::MenuItem("Quit", "ESC"))
				ret = UPDATE_STOP;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
            ImGui::MenuItem("Console", "F1", &console->active);
            ImGui::MenuItem("Hierarchy", "F2", &tree->active);
            ImGui::MenuItem("Properties", "F3", &props->active);
            ImGui::MenuItem("Configuration", "F4", &props->active);

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("Gui Demo"))
				showcase = !showcase;

			if (ImGui::MenuItem("About"))
				about->SwitchActive();

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (ImGui::BeginPopup("About"))
	{
		ImGui::TextUnformatted("Info about the EDU Engine");
		ImGui::EndPopup();
	}

	// Draw all active panels
	for (vector<Panel*>::iterator it = panels.begin(); it != panels.end(); ++it)
	{
		Panel* panel = (*it);

		if (App->input->GetKey(panel->GetShortCut()) == KEY_DOWN)
			panel->SwitchActive();

		if(panel->IsActive())
			panel->Draw();
	}

	// Link tree and property panel
	props->selected = (GameObject*) tree->selected;

	if (load_file_active)
		LoadFile();

	// Show showcase ? 
	if(showcase)
		ImGui::ShowTestWindow(&showcase);

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

    ImGui_ImplSdlGL3_Shutdown();

	return true;
}

void ModuleEditor::HandleInput(SDL_Event* event)
{
    ImGui_ImplSdlGL3_ProcessEvent(event);
}

void ModuleEditor::Draw()
{
	// Debug Draw on selected GameObject
	BeginDebugDraw();

	if (props->selected != nullptr)
		props->selected->OnDebugDraw();

	EndDebugDraw();
	ImGui::Render();
}

bool ModuleEditor::UsingInput() const
{
	return in_modal || ImGui::IsMouseHoveringAnyWindow() || (ImGui::IsAnyItemActive() && ImGui::IsMouseDragging());
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

void ModuleEditor::LoadFile(const char* filter_extension)
{
	ImGui::OpenPopup("Load File");
	if (ImGui::BeginPopupModal("Load File", &load_file_active, ImGuiWindowFlags_AlwaysAutoResize))
	{
		in_modal = true;

        ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
        ImGui::BeginChild("File Browser", ImVec2(0,300), true);
		DrawDirectoryRecursive("", filter_extension);
        ImGui::EndChild();
        ImGui::PopStyleVar();

		ImGui::PushItemWidth(250.f);
		ImGui::InputText("##file_selector", selected_file, FILE_MAX, ImGuiInputTextFlags_AutoSelectAll);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImGui::Button("Ok", ImVec2(50, 20));
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(50, 20)))
		{
			load_file_active = false;
			ImGui::CloseCurrentPopup();
		}


		ImGui::EndPopup();
	}
	else
		in_modal = false;
}

void ModuleEditor::DrawDirectoryRecursive(const char* directory, const char* filter_extension) 
{
	vector<string> files;
	vector<string> dirs;

	std::string dir(directory);
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
			if (ImGui::IsItemClicked())
				sprintf_s(selected_file, FILE_MAX, "%s%s", dir.c_str(), str.c_str());

			ImGui::TreePop();
		}
	}
}
