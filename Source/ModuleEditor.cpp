#include "Globals.h"
#include "Application.h"
#include "ModuleEditor.h"
#include "ModuleWindow.h"
#include "ModuleFileSystem.h"
#include "Config.h"

#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_sdl_gl3.h"

#include "OpenGL.h"

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

	return true;
}

update_status ModuleEditor::PreUpdate(float dt)
{
    ImGui_ImplSdlGL3_NewFrame(App->window->window);
	return UPDATE_CONTINUE;
}

update_status ModuleEditor::Update(float dt)
{
	// Console log
	panel_log->Draw("Console");

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleEditor::CleanUp()
{
	LOG("Freeing editor gui");
					  
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
