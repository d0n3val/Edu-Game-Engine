#include "Globals.h"
#include "Application.h"
#include "ModuleEditor.h"
#include "ModuleWindow.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "Config.h"

#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_sdl_gl3.h"

#include "OpenGL.h"

using namespace std;

struct EditorLog
{
    ImGuiTextBuffer     Buf;
    bool                ScrollToBottom;

    void    Clear()     { Buf.clear(); }

    void    AddLog(const char* fmt)
    {
        Buf.append(fmt);
        ScrollToBottom = true;
    }

    void    Draw(const char* title, bool* p_opened = NULL)
    {
        ImGui::Begin(title, p_opened, ImGuiWindowFlags_NoFocusOnAppearing );
        ImGui::TextUnformatted(Buf.begin());
        if (ScrollToBottom)
            ImGui::SetScrollHere(1.0f);
        ScrollToBottom = false;
        ImGui::End();
    }
};

struct EditorGameObjects
{
	uint node = 0;
	char name[80];

    void    Draw(const char* title, bool* p_opened = NULL)
    {
		node = 0;
        ImGui::Begin("GameObjects Hierarchy", p_opened, ImGuiWindowFlags_NoFocusOnAppearing );
		RecursiveDraw(App->scene->GetRoot());
        ImGui::End();
    }

	void RecursiveDraw(const GameObject* go)
	{
		sprintf_s(name, 80, "%s##node_%i", go->name.c_str(), node++);
		if (ImGui::TreeNode(name))
		{
			for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
				RecursiveDraw(*it);
			ImGui::TreePop();
		}
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

	panel_log = new EditorLog();
	panel_gameObjects = new EditorGameObjects();

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
	
	static bool show_console = false;
	static bool show_gameobjects = false;

	// Main Menu
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
				show_console = !show_console;

			if (ImGui::MenuItem("Game Objects", "F2"))
				show_gameobjects = !show_gameobjects;

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
	if(show_console)
		panel_log->Draw("Console");

	if (show_gameobjects)
		panel_gameObjects->Draw("");

	return ret;
}

// Called before quitting
bool ModuleEditor::CleanUp()
{
	LOG("Freeing editor gui");
					  
	RELEASE(panel_gameObjects);
	RELEASE(panel_log);
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
	if(panel_log != nullptr)
		panel_log->AddLog(entry);
}
