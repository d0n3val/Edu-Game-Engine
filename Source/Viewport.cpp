#include "Globals.h"
#include "Viewport.h"

#include "SceneViewport.h"
#include "StateViewport.h"

#include "Application.h"

#include "ModuleRenderer.h"
#include "ModuleRenderer3D.h"
#include "ModuleInput.h"
#include "ModuleDebugDraw.h"
#include "ModuleEditor.h"
#include "ModuleEditorCamera.h"
#include "ModuleHints.h"
#include "ModulePrograms.h"

#include "GameObject.h"

#include "PointLight.h"
#include "SpotLight.h"

#include "ComponentCamera.h"
#include "ComponentAnimation.h"

#include "Config.h"
#include "DebugDraw.h"

#include "ImGui.h"
#include "GL/glew.h"

#include "mmgr/mmgr.h"

//namespace ed = ax::NodeEditor;

//extern ed::EditorContext* g_Context;

Viewport::Viewport()
{
    scene = new SceneViewport;
    state = new StateViewport;
}

Viewport::~Viewport()
{
    delete scene;
    delete state;
}

void Viewport::Draw(ComponentCamera* camera)
{
	int posx = App->editor->GetPosX(ModuleEditor::TabPanelLeft) + App->editor->GetWidth(ModuleEditor::TabPanelLeft);
	int posy = 21;
	int width = App->editor->GetPosX(ModuleEditor::TabPanelRight) - posx;
	int height = App->editor->GetPosY(ModuleEditor::TabPanelBottom) - 21;

    ImGui::SetNextWindowPos(ImVec2(float(posx), float(posy)));
    ImGui::SetNextWindowSize(ImVec2(float(width), float(height)));

    bool active = true;
	if (ImGui::Begin("Viewport", &active, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing))
	{
        if (ImGui::BeginTabBar("##viewport_tabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Scene"))
            {
                scene->Draw(camera);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("StateMachine"))
            {
                ComponentAnimation* animation = nullptr;
                if(App->editor->selection_type == ModuleEditor::SelectionGameObject && App->editor->selected.go != nullptr)
                {
					animation = static_cast<ComponentAnimation*>(App->editor->selected.go->FindFirstComponent(Component::Animation));
                }
                state->Draw(animation);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
	ImGui::End();
}

void Viewport::Save(Config* config) const
{
    scene->Save(config);
}

void Viewport::Load(Config* config)
{
    scene->Load(config);
}

