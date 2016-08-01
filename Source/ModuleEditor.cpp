#include "Globals.h"
#include "Application.h"
#include "ModuleEditor.h"
#include "ModuleWindow.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"
#include "ModuleInput.h"
#include "GameObject.h"
#include "Config.h"
#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_sdl_gl3.h"
#include "OpenGL.h"

#include "Panel.h"
#include "PanelConsole.h"
#include "PanelGOTree.h"
#include "PanelProperties.h"

using namespace std;

struct EditorGameObjectProperties
{
	bool opened = false;

	void Draw()
	{
        ImGui::Begin("Properties", &opened, ImGuiWindowFlags_NoFocusOnAppearing );
        ImGui::End();
	}
};


ModuleEditor::ModuleEditor(bool start_enabled) : Module("Editor", start_enabled)
{
}

// Destructor
ModuleEditor::~ModuleEditor()
{
}

// Called before render is available
bool ModuleEditor::Init(Config* config)
{
	LOG("Init editor gui with imgui lib");

    ImGui_ImplSdlGL3_Init(App->window->window);

	// create all panels
	panels.push_back(console = new PanelConsole());
	panels.push_back(tree = new PanelGOTree());
	panels.push_back(props = new PanelProperties());

	return true;
}

update_status ModuleEditor::PreUpdate(float dt)
{
    ImGui_ImplSdlGL3_NewFrame(App->window->window);
	return UPDATE_CONTINUE;
}

update_status ModuleEditor::Update(float dt)
{
	update_status ret = UPDATE_CONTINUE;
	
	// Main menu GUI
	if (ImGui::BeginMainMenuBar())
	{
		bool selected = false;
		if (ImGui::BeginMenu("File"))
		{
			ImGui::MenuItem("New ...");
			ImGui::MenuItem("Load ...");
			ImGui::MenuItem("Save ...");
			if (ImGui::MenuItem("Quit", "ESC"))
				ret = UPDATE_STOP;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Console", "F1"))
				console->SwitchActive();

			if (ImGui::MenuItem("Hierarchy", "F2"))
				tree->SwitchActive();

			if (ImGui::MenuItem("Properties", "F3"))
				props->SwitchActive();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About"))
				ImGui::OpenPopup("About");

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (ImGui::BeginPopup("About"))
	{
		ImGui::TextUnformatted("Info about the EDU Engine");
		ImGui::EndPopup();
	}

	// Console log
	for (vector<Panel*>::iterator it = panels.begin(); it != panels.end(); ++it)
	{
		Panel* panel = (*it);

		if (App->input->GetKey(panel->GetShortCut()) == KEY_DOWN)
			panel->SwitchActive();

		if(panel->IsActive())
			panel->Draw();
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

	console = nullptr;
	tree = nullptr;
	props = nullptr;

    ImGui_ImplSdlGL3_Shutdown();

	return true;
}

void ModuleEditor::HandleInput(SDL_Event* event)
{
    ImGui_ImplSdlGL3_ProcessEvent(event);
}

void ModuleEditor::Draw()
{
	ImGui::Render();
}

bool ModuleEditor::IsHoveringGui()
{
	return ImGui::IsMouseHoveringAnyWindow();
}

void ModuleEditor::Log(const char * entry)
{
	if(console != nullptr)
		console->AddLog(entry);
}
