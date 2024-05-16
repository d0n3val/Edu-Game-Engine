#include "PanelProperties.h"
#include "Application.h"
#include "Imgui/imgui.h"
#include "Imgui/imguizmo.h"
#include "GameObject.h"
#include "Component.h"
#include "ComponentAudioSource.h"
#include "ComponentAudioListener.h"
#include "ComponentCamera.h"
#include "ComponentPath.h"
#include "ComponentRigidBody.h"
#include "ComponentSteering.h"
#include "ComponentMeshRenderer.h"
#include "ComponentAnimation.h"
#include "ComponentRootMotion.h"
#include "ComponentParticleSystem.h"
#include "ComponentTrail.h"
#include "ComponentLine.h"
#include "ComponentGrass.h"
#include "ComponentDecal.h"
#include "ComponentSpotCone.h"
#include "ModuleLevelManager.h"
#include "ModuleEditor.h"
#include "ModuleResources.h"
#include "ModuleFileSystem.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditorCamera.h"
#include "ModuleWindow.h"
#include "ModuleHints.h"
#include "ModulePrograms.h"
#include "ModuleInput.h"
#include "ModuleRenderer.h"
#include "DebugDraw.h"
#include "ResourceTexture.h"
#include "ResourceMesh.h"
#include "ResourceMaterial.h"
#include "ResourceStateMachine.h"
#include "ResourceAudio.h"
#include "ResourceAnimation.h"
#include "PanelResources.h"
#include "PanelGOTree.h"
#include "PerlinProperties.h"
#include "Viewport.h"
#include "SceneViewport.h"
#include "BatchManager.h"
#include "SkyboxRollout.h"
#include "LightManager.h"

#include "DirLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "QuadLight.h"
#include "SphereLight.h"
#include "TubeLight.h"
#include "LocalIBLLight.h"

#include "OpenGL.h"

#include "visit_variant.h"

#include <list>
#include <algorithm>
#include <variant>


#include "Leaks.h"

using namespace std;

#undef min
#undef max

// ---------------------------------------------------------
PanelProperties::PanelProperties() : Panel("Properties")
{
	width = default_width;
	height = default_height;
	posx = default_posx;
	posy = default_posy;
    perlin = std::make_unique<PerlinProperties>();
    skybox = std::make_unique<SkyboxRollout>();
}

// ---------------------------------------------------------
PanelProperties::~PanelProperties()
{}

// ---------------------------------------------------------
void PanelProperties::Draw()
{
    std::visit(overload {
        [this](GameObject* go)      { DrawGameObject(go, nullptr);      },
        [this](DirLight* light)     { DrawDirLight(light);     },
        [this](PointLight* light)   { DrawPointLight(light);   },
        [this](SpotLight* light)    { DrawSpotLight(light);    },
        [this](QuadLight* light)    { DrawQuadLight(light);    },
        [this](SphereLight* light)  { DrawSphereLight(light);    },
        [this](TubeLight* light)       { DrawTubeLight(light);     },
        [this](LocalIBLLight* light)   { DrawLocalIBLLight(light); },
        [this](IBLData* sky)         { skybox->DrawProperties(sky);},
        [this](ComponentMeshRenderer* renderer) { DrawGameObject(renderer->GetGameObject(), renderer); },
        }, App->editor->GetSelection());

    show_texture.Display();
    selectTexture.Display();
    newMeshDlg.Display();
}

// ---------------------------------------------------------
void PanelProperties::DrawDirLight(DirLight* light)
{
    if (ImGui::CollapsingHeader("Directional light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        float3 color = light->GetColor();
        if(ImGui::ColorEdit3("color", (float*)&color))
        {
            light->SetColor(color);
        }

        float intensity = light->GetIntensity();
        if(ImGui::SliderFloat("intensity", &intensity, 0.0f, 100.0f))
        {
            light->SetIntensity(intensity);
        }

        float azimuthal = light->GetAzimuthal();
        if(ImGui::SliderAngle("azimuthal", &azimuthal, 0.0f, 360.0f))
        {
            light->SetAzimuthal(azimuthal);
        }

        float polar = light->GetPolar();
        if(ImGui::SliderAngle("polar", &polar, 0.0f, 180.0f))
        {
            light->SetPolar(polar);
        }

        float anisotropy = light->GetAnisotropy();
        if(ImGui::SliderFloat("anisotropy", &anisotropy, -1.0f, 1.0f, "%.4f"))
        {
            light->SetAnisotropy(anisotropy);
        }

    }
}

// ---------------------------------------------------------
void PanelProperties::DrawPointLight(PointLight* light)
{
    if (ImGui::CollapsingHeader("Point light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        float3 color = light->GetColor();
        if(ImGui::ColorEdit3("color", (float*)&color))
        {
            light->SetColor(color);
        }

        float intensity = light->GetIntensity();
        if(ImGui::SliderFloat("intensity", &intensity, 0.0f, 100.0f))
        {
            light->SetIntensity(intensity);
        }

        ImGui::Separator();

        App->renderer3D->viewport->GetScene()->DrawGuizmoProperties(light);

        ImGui::Separator();

        float radius = light->GetRadius();
        if(ImGui::InputFloat("radius", &radius, 0.01f, 0.1f, "%.9f"))
        {
            light->SetRadius(radius);
        }

        float anisotropy = light->GetAnisotropy();
        if (ImGui::SliderFloat("anisotropy", &anisotropy, -1.0f, 1.0f, "%.4f"))
        {
            light->SetAnisotropy(anisotropy);
        }

        bool enabled = light->GetEnabled();
        if(ImGui::Checkbox("Enabled", &enabled))
        {
            light->SetEnabled(enabled);
        }

    }
}

// ---------------------------------------------------------
void PanelProperties::DrawSpotLight(SpotLight* light)
{
    if (ImGui::CollapsingHeader("Spot light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        float3 color = light->GetColor();
        if(ImGui::ColorEdit3("color", (float*)&color))
        {
            light->SetColor(color);
        }

        float intensity = light->GetIntensity();
        if(ImGui::SliderFloat("intensity", &intensity, 0.0f, 100.0f))
        {
            light->SetIntensity(intensity);
        }

        ImGui::Separator();

        App->renderer3D->viewport->GetScene()->DrawGuizmoProperties(light);

        ImGui::Separator();

        float inner = light->GetInnerCutoff();
        if(ImGui::SliderAngle("inner cutoff", &inner, 0.0f, 90.0f))
        {
            light->SetInnerCutoff(inner);
        }

        float outter = light->GetOutterCutoff();
        if(ImGui::SliderAngle("outter cutoff", &outter, 0.0f, 90.0f))
        {
            light->SetOutterCutoff(outter);
        }

        ImGui::Separator();

        float distance = light->GetMaxDistance();
        if(ImGui::InputFloat("max distance", &distance, 0.1f, 0.1f, "%.9f"))
        {
            light->SetMaxDistance(distance);
        }

        distance = light->GetMinDistance();
        if (ImGui::InputFloat("min distance", &distance, 0.1f, 0.1f, "%.9f"))
        {
            light->SetMinDistance(distance);
        }

        float anisotropy = light->GetAnisotropy();
        if (ImGui::SliderFloat("anisotropy", &anisotropy, -1.0f, 1.0f, "%.4f"))
        {
            light->SetAnisotropy(anisotropy);
        }

        bool cast = light->GetCastShadows();
        if(ImGui::Checkbox("Casts shadows", &cast))
        {
            light->SetCastShadows(cast);
        }

        int size = light->GetShadowSize();
        if (ImGui::InputInt("Shadow size", &size))
        {
            light->SetShadowSize(size);
        }

        bool enabled = light->GetEnabled();
        if(ImGui::Checkbox("Enabled", &enabled))
        {
            light->SetEnabled(enabled);
        }

    }
}

// ---------------------------------------------------------
void PanelProperties::DrawQuadLight(QuadLight *light)
{
    if (ImGui::CollapsingHeader("Quad light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        float3 color = light->GetColor();
        if(ImGui::ColorEdit3("color", (float*)&color))
        {
            light->SetColor(color);
        }

        float intensity = light->GetIntensity();
        if(ImGui::SliderFloat("intensity", &intensity, 0.0f, 100.0f))
        {
            light->SetIntensity(intensity);
        }

        ImGui::Separator();

        App->renderer3D->viewport->GetScene()->DrawGuizmoProperties(light);

        ImGui::Separator();

        bool enabled = light->GetEnabled();
        if(ImGui::Checkbox("Enabled", &enabled))
        {
            light->SetEnabled(enabled);
        }
    }

}

// ---------------------------------------------------------
void PanelProperties::DrawSphereLight(SphereLight *light)
{
    if (ImGui::CollapsingHeader("Sphere light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        float3 color = light->GetColor();
        if(ImGui::ColorEdit3("color", (float*)&color))
        {
            light->SetColor(color);
        }

        float intensity = light->GetIntensity();
        if(ImGui::SliderFloat("intensity", &intensity, 0.0f, 100.0f))
        {
            light->SetIntensity(intensity);
        }

        float anisotropy = light->GetAnisotropy();
        if (ImGui::SliderFloat("anisotropy", &anisotropy, -1.0f, 1.0f, "%.4f"))
        {
            light->SetAnisotropy(anisotropy);
        }

        ImGui::Separator();

        App->renderer3D->viewport->GetScene()->DrawGuizmoProperties(light);

        ImGui::Separator();

        float radius = light->GetRadius();
        if(ImGui::InputFloat("radius", &radius, 0.01f, 0.1f, "%.9f"))
        {
            light->SetRadius(radius);
        }

        bool enabled = light->GetEnabled();
        if(ImGui::Checkbox("Enabled", &enabled))
        {
            light->SetEnabled(enabled);
        }
    }
}

// ---------------------------------------------------------
void PanelProperties::DrawTubeLight(TubeLight *light)
{
    if (ImGui::CollapsingHeader("Tube light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        float3 color = light->GetColor();
        if(ImGui::ColorEdit3("color", (float*)&color))
        {
            light->SetColor(color);
        }

        float intensity = light->GetIntensity();
        if(ImGui::SliderFloat("intensity", &intensity, 0.0f, 100.0f))
        {
            light->SetIntensity(intensity);
        }

        ImGui::Separator();
        float anisotropy = light->GetAnisotropy();
        if (ImGui::SliderFloat("anisotropy", &anisotropy, -1.0f, 1.0f, "%.4f"))
        {
            light->SetAnisotropy(anisotropy);
        }


        App->renderer3D->viewport->GetScene()->DrawGuizmoProperties(light);

        ImGui::Separator();

        // Position and rotation ??? 
        float height = light->GetHeight();
        if (ImGui::InputFloat("height", &height, 0.01f, 0.1f, "%.9f"))
        {
            light->SetHeight(height);
        }


        float radius = light->GetRadius();
        if(ImGui::InputFloat("radius", &radius, 0.01f, 0.1f, "%.9f"))
        {
            light->SetRadius(radius);
        }

        radius = light->GetAttRadius();
        if(ImGui::InputFloat("attenuation radius", &radius, 0.01f, 0.1f, "%.9f"))
        {
            light->SetAttRadius(radius);
        }


        bool enabled = light->GetEnabled();
        if(ImGui::Checkbox("Enabled", &enabled))
        {
            light->SetEnabled(enabled);
        }
    }
}

// ---------------------------------------------------------
void PanelProperties::DrawLocalIBLLight(LocalIBLLight *light)
{
    if (ImGui::CollapsingHeader("Local IBL light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        bool enable = light->GetEnabled();
        if(ImGui::Checkbox("Enabled", &enable))
        {
            light->SetEnabled(enable);
        }

        App->renderer3D->viewport->GetScene()->DrawGuizmoProperties(light);
        skybox->DrawProperties(&light->getIBLData());
        if(ImGui::Button("Generate"))
        {
            light->generate();
        }

        AABB parallax = light->GetParallaxAABB();
        if(ImGui::DragFloat3("Parallax min", (float*)& parallax.minPoint, 0.001f))
        {
            light->SetParallaxAABB(parallax);
        }

        if(ImGui::DragFloat3("Parallax max", (float*) & parallax.maxPoint, 0.001f))
        {
            light->SetParallaxAABB(parallax);
        }

        AABB influence = light->GetInfluenceAABB();
        if(ImGui::DragFloat3("Influence min", (float*)& influence.minPoint, 0.001f))
        {
            light->SetInfluenceAABB(influence);
        }

        if(ImGui::DragFloat3("Influence max", (float*) & influence.maxPoint, 0.001f))
        {
            light->SetInfluenceAABB(influence);
        }
        
        float farPlane = light->GetFarPlane();
        if(ImGui::DragFloat("Far plane", &farPlane, 0.1f, 0.0f, 10000000.0f))
        {
            light->SetFarPlane(farPlane);
        }

        uint resolution = light->GetResolution();
        if (ImGui::DragInt("Resolution", (int*) & resolution))
        {
            light->SetResolution(resolution);
        }

        uint numSamples = light->GetNumSamples();
        if (ImGui::DragInt("Num samples", (int*)&numSamples))
        {
            light->SetNumSamples(numSamples);
        }

        uint roughnessLevels = light->GetNumRoughnessLevels();
        if (ImGui::DragInt("Roughness levels", (int*)&roughnessLevels))
        {
            light->SetNumRoughnessLevels(roughnessLevels);
        }
    }

}

// ---------------------------------------------------------
void PanelProperties::DrawGameObject(GameObject* go, Component* component)
{
    if (ImGui::BeginMenu("Options"))
    {
        if (ImGui::MenuItem("Reset Transform", nullptr, nullptr, (go != nullptr)))
        {
            go->SetLocalPosition(float3::zero);
            go->SetLocalScale(float3::one);
            go->SetLocalRotation(Quat::identity);
        }

        static_assert(Component::Types::Unknown == 16, "code needs update");
        if (ImGui::BeginMenu("New Component", (go != nullptr)))
        {
            if (ImGui::MenuItem("Audio Listener"))
                go->CreateComponent(Component::Types::AudioListener);
            if (ImGui::MenuItem("Audio Source"))
                go->CreateComponent(Component::Types::AudioSource);
			if (ImGui::MenuItem("MeshRenderer"))
				go->CreateComponent(Component::Types::MeshRenderer);
            if (ImGui::MenuItem("Camera"))
                go->CreateComponent(Component::Types::Camera);
            if (ImGui::MenuItem("RigidBody"))
                go->CreateComponent(Component::Types::RigidBody);
            if (ImGui::MenuItem("Animation"))
                go->CreateComponent(Component::Types::Animation);
            if (ImGui::MenuItem("Steering"))
                go->CreateComponent(Component::Types::Steering);
            if (ImGui::MenuItem("Path"))
                go->CreateComponent(Component::Types::Path);
			if (ImGui::MenuItem("RootMotion"))
				go->CreateComponent(Component::Types::RootMotion);
			if (ImGui::MenuItem("SimpleCharacter"))
				go->CreateComponent(Component::Types::CharacterController);
			if (ImGui::MenuItem("ParticleSystem"))
				go->CreateComponent(Component::Types::ParticleSystem);
			if (ImGui::MenuItem("Trail"))
				go->CreateComponent(Component::Types::Trail);
            if (ImGui::MenuItem("Line"))
                go->CreateComponent(Component::Types::Line);
			if (ImGui::MenuItem("Grass"))
				go->CreateComponent(Component::Types::Grass);
			if (ImGui::MenuItem("Decal"))
				go->CreateComponent(Component::Types::Decal);
            if (ImGui::MenuItem("SpotCone"))
                go->CreateComponent(Component::Types::SpotCone);
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    if (go != nullptr )
    {

        // Active check box
        bool active = go->IsActive();
        ImGui::Checkbox(" ", &active);
        go->SetActive(active);

        if(ImGui::IsItemHovered())
            ImGui::SetTooltip("UID: %u", go->GetUID());

        ImGui::SameLine();

        // Text Input for the name
        char name[50];
        strcpy_s(name, 50, go->name.c_str());
        if (ImGui::InputText("Name", name, 50,
                    ImGuiInputTextFlags_AutoSelectAll |
                    ImGuiInputTextFlags_EnterReturnsTrue))
            go->name = name;

        // Transform section ============================================
        if (ImGui::CollapsingHeader("Local Transformation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            App->renderer3D->viewport->GetScene()->DrawGuizmoProperties(go);

            ImGui::Text("Bounding Box: ");
            ImGui::SameLine();
            if (go->global_bbox.IsFinite())
            {
                float3 size = go->GetLocalBBox().Size();
                ImGui::TextColored(IMGUI_YELLOW, "%.2f %.2f %.2f", size.x, size.y, size.x);
            }
            else
                ImGui::TextColored(IMGUI_YELLOW, "- not generated -");

            ImGui::Text("Velocity: ");
            ImGui::SameLine();
            float3 vel = go->GetVelocity();
            ImGui::TextColored(IMGUI_YELLOW, "%.2f %.2f %.2f (%.2f m/s)", vel.x, vel.y, vel.x, vel.Length());
        }

        // Iterate all components and draw
        static_assert(Component::Types::Unknown == 16, "code needs update");
        for (list<Component*>::iterator it = go->components.begin(); it != go->components.end(); ++it)
        {
            ImGui::PushID(*it);
            if (InitComponentDraw(*it, (*it)->GetTypeStr(), component == nullptr || *it == component))
            {
                switch ((*it)->GetType())
                {
					case Component::Types::MeshRenderer:
						DrawMeshRendererComponent((ComponentMeshRenderer*)(*it));
						break;
                    case Component::Types::AudioSource:
                        DrawAudioSourceComponent((ComponentAudioSource*)(*it));
                        break;
                    case Component::Types::AudioListener:
                        DrawAudioListenerComponent((ComponentAudioListener*)(*it));
                        break;
                    case Component::Types::Camera:
                        DrawCameraComponent((ComponentCamera*)(*it));
                        break;
                    case Component::Types::RigidBody:
                        ((ComponentRigidBody*)(*it))->DrawEditor();
                        break;
                    case Component::Types::Animation:
                        DrawAnimationComponent(static_cast<ComponentAnimation*>(*it));
                        break;
                    case Component::Types::Steering:
                        ((ComponentSteering*)(*it))->DrawEditor();
                        break;
                    case Component::Types::Path:
                        ((ComponentPath*)(*it))->DrawEditor();
                        break;
					case Component::Types::RootMotion:
						DrawRootMotionComponent(static_cast<ComponentRootMotion*>(*it));
						break;
					case Component::Types::ParticleSystem:
						DrawParticleSystemComponent(static_cast<ComponentParticleSystem*>(*it));
                        break;
					case Component::Types::Trail:
						DrawTrailComponent(static_cast<ComponentTrail*>(*it));
                        break;
                    case Component::Types::Line:
                        DrawLineComponent(static_cast<ComponentLine*>(*it));
                        break;
					case Component::Types::Grass:
						DrawGrassComponent(static_cast<ComponentGrass*>(*it));
                        break;
					case Component::Types::Decal:
						DrawDecalComponent(static_cast<ComponentDecal*>(*it));
                        break;
                    case Component::Types::SpotCone:
                        DrawSpotConeComponent(static_cast<ComponentSpotCone*>(*it));
                        break;
				}
            }
            ImGui::PopID();
        }

    }

}

UID PanelProperties::PickResource(UID resource, int type)
{
	UID ret = 0;

	if (resource > 0)
	{
		const Resource* res = App->resources->Get(resource);

		std::string file;
		ImGui::TextColored(IMGUI_YELLOW, "%s", res->GetFile());
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("Type: %s", res->GetTypeStr());
			ImGui::Text("UID: %llu", res->GetUID());
			ImGui::Text("Lib: %s", res->GetExportedFile());
			ImGui::EndTooltip();
		}
		if (ImGui::Button("Change Resource"))
			ImGui::OpenPopup("Load Resource");
	}
	else
	{
		if (ImGui::Button("Attach Resource"))
			ImGui::OpenPopup("Load Resource");
	}

	if (ImGui::BeginPopup("Load Resource"))
	{
        ret = DrawResourceType((Resource::Type) type, false);
        ImGui::EndPopup();
	}

	return ret;
}

UID PanelProperties::PickResourceModal(int type)
{
    char tmp[128];
    sprintf_s(tmp, 127, "Select %s", Resource::GetTypeStr(Resource::Type(type)));

    if (ImGui::Button(tmp))
    {
        ImGui::OpenPopup(tmp);
    }

    return OpenResourceModal(type, tmp);
}

UID PanelProperties::CreateNewMesh()
{
    if (ImGui::Button("Create New Mesh"))
    {
        newMeshDlg.Open();
    }

    return newMeshDlg.getMesh();
}

UID PanelProperties::OpenResourceModal(int type, const char* popup_name)
{
	UID new_res = 0;

    ImGui::SetNextWindowSize(ImVec2(420,300));
    if (ImGui::BeginPopupModal(popup_name, nullptr, ImGuiWindowFlags_NoResize))
    {
        if(ImGui::BeginChild("Canvas", ImVec2(400, 240), true, ImGuiWindowFlags_NoMove))
        {
            UID r = 0;
            r = DrawResourceType(Resource::Type(type), true);

            if(r != 0)
            {
                ImGui::CloseCurrentPopup();
                new_res = r;
            }
        }
        ImGui::EndChild();

        ImGui::Indent(272);
        if(ImGui::Button("Close", ImVec2(128, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    return new_res;
}

const GameObject* PanelProperties::PickGameObject(const GameObject* current) const
{
	const GameObject* ret = nullptr;

	ImGui::Text("Current: ");
	ImGui::SameLine();
	ImGui::TextColored(IMGUI_YELLOW, "%s", (current) ? current->name.c_str() : "- not assigned -");
	if (ImGui::Button("Pick Another"))
		ImGui::OpenPopup("Pick GameObject");

	if (ImGui::BeginPopup("Pick GameObject"))
	{
		// Draw the tree
		GameObject* root = App->level->GetRoot();
		for (list<GameObject*>::const_iterator it = root->childs.begin(); it != root->childs.end(); ++it)
			RecursiveDrawTree(*it, &ret);

		ImGui::EndPopup();
	}

	return ret;
}

void PanelProperties::RecursiveDrawTree(const GameObject * go, const GameObject** selected) const
{
	if (ImGui::TreeNodeEx(go, (go->childs.size() > 0) ? 0 : ImGuiTreeNodeFlags_Bullet, go->name.c_str())) 
	{
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) && ImGui::IsMouseDoubleClicked(0))
		{
			*selected = go;
			ImGui::CloseCurrentPopup();
		}

		for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
			RecursiveDrawTree(*it, selected);

		ImGui::TreePop();
	}
	else
	{
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) && ImGui::IsMouseDoubleClicked(0))
		{
			*selected = go;
			ImGui::CloseCurrentPopup();
		}
	}
}

bool PanelProperties::InitComponentDraw(Component* component, const char * name, bool opened)
{
	bool ret = false;

	if (ImGui::CollapsingHeader(name, opened ? ImGuiTreeNodeFlags_DefaultOpen : 0))
	{
		bool active = component->IsActive();
		if(ImGui::Checkbox("Active", &active))
			component->SetActive(active);
		ImGui::SameLine();
		if (ImGui::SmallButton("Delete Component"))
			component->flag_for_removal = true;
		ret = true;
	}

	return ret;
}

void PanelProperties::DrawBatchProperties(ComponentMeshRenderer* component)
{
    const ResourceMesh* mesh = component->GetMeshRes();
    if(mesh)
    {
        BatchManager* batch_manager = App->renderer->GetBatchManager();

        std::vector<HashString> tag_names;

        batch_manager->FillBatchNames(tag_names);
        const HashString& batch_name = component->GetBatchName();

        std::vector<HashString>::iterator it = std::find(tag_names.begin(), tag_names.end(), batch_name);

        int selected_index = it == tag_names.end() ? int(tag_names.size()) : int(it - tag_names.begin());

        tag_names.push_back(HashString("empty"));

        bool (*items_getter)(void* data, int idx, const char** out_text) = [](void* data, int idx, const char** out_text) -> bool 
        { 
            std::vector<HashString>& v = (*reinterpret_cast<std::vector<HashString>*>(data));
            if(idx < int(v.size()))
            {
                *out_text = v[idx].C_str();
                return true;
            }

            return false;
        };

        if (ImGui::Combo("Batch", &selected_index, items_getter, &tag_names, int(tag_names.size())))
        {
            if (selected_index + 1 == tag_names.size())
            {
                component->SetBatchName(HashString());
            }
            else
            {
                component->SetBatchName(tag_names[selected_index]);
            }
        }

        if(ImGui::Button("New batch"))
        {
            ImGui::OpenPopup("Batch name");
        }

        if (ImGui::BeginPopupModal("Batch name", nullptr, ImGuiWindowFlags_NoResize))
        {
            static char tmp[256];
            static bool init = true;

            if(ImGui::BeginChild("Canvas", ImVec2(250, 90), true, ImGuiWindowFlags_NoMove))
            {
                if(init)
                {
                    const char* name = component->GetBatchName().C_str();
                    if (name == nullptr)
                    {
                        tmp[0] = 0;
                    }
                    else
                    {
                        strcpy_s(&tmp[0], 255, name);
                    }

                    init = false;
                }

                ImGui::InputText("Batch name", tmp, 256);
            }
            ImGui::EndChild();

            ImGui::Indent(122);
            if(ImGui::Button("Ok", ImVec2(60, 0)))
            {
                component->SetBatchName(HashString(&tmp[0]));
                init = true;

                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if(ImGui::Button("Cancel", ImVec2(60, 0)))
            {
                init = true;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
}

void PanelProperties::DrawMeshRendererComponent(ComponentMeshRenderer* component)
{
    // Mesh

	const ResourceMesh* res = component->GetMeshRes();

    DrawMesh(res);

    DrawBatchProperties(component);

    bool visible = component->GetVisible();
    if (ImGui::Checkbox("Visible", &visible))
    {
        component->SetVisible(visible);
    }

    if (ImGui::CollapsingHeader("Morph Targets", 0))
    {
        for (uint i = 0; i < res->GetNumMorphTargets(); ++i)
        {
            float weight = component->GetMorphTargetWeight(i);

            char tmp[128];
            sprintf_s(tmp, 127, "Morph %d", i);

            if (ImGui::DragFloat(tmp, &weight, 0.01f, 0.0f, 1.0f))
            {
                component->SetMorphTargetWeight(i, weight);
            }
        }
    }

    UID new_res = PickResourceModal(Resource::mesh);

    if (new_res > 0)
    {
        component->SetMeshRes(new_res);
    }

    new_res = CreateNewMesh();

    if (new_res > 0)
    {
        component->SetMeshRes(new_res);
    }


    ImGui::Separator();

    const char* names[ComponentMeshRenderer::RENDER_COUNT] = { "Opaque", "Transparent" };

    int index = int(component->RenderMode());
    if(ImGui::Combo("Render mode", &index, names, int(ComponentMeshRenderer::RENDER_COUNT)))
    {
		component->SetRenderMode(ComponentMeshRenderer::ERenderMode(index));
    }

    ImGui::Separator();

    bool debug_draw = component->GetDDTangent();
    if(ImGui::Checkbox("Debug draw tangent space", &debug_draw))
    {
        component->SetDDTangent(debug_draw);
    }

    bool cast_shadows = component->CastShadows();
    if(ImGui::Checkbox("Cast shadows", &cast_shadows))
    {
        component->SetCastShadows(cast_shadows);
    }

    ImGui::Separator();

    // Material

    ResourceMaterial* mat_res = component->GetMaterialRes();

    new_res = PickResourceModal(Resource::material);

    ImGui::SameLine();
    if (ImGui::Button("New material"))
    {
        ResourceMaterial* material = static_cast<ResourceMaterial*>(App->resources->CreateNewResource(Resource::material, 0));

        bool save_ok = material->Save();

        if (save_ok)
        {
            new_res = material->GetUID();
        }
    }

    if(new_res > 0)
    {
		component->SetMaterialRes(new_res);
    }
    else if(mat_res)
	{
		DrawMaterialResource(mat_res, component->GetMeshRes());
    }
}

void PanelProperties::DrawAudioSourceComponent(ComponentAudioSource * component)
{
	UID new_res = PickResource(component->GetResourceUID(), Resource::audio);
	if (new_res > 0)
		component->SetResource(new_res);

	const ResourceAudio* res = (const ResourceAudio*) component->GetResource();
	const char* file = (res) ? res->GetFile() : nullptr;

	ImGui::Text("File: ");
	ImGui::SameLine();
	ImGui::TextColored(IMGUI_YELLOW, (file) ? file : "No file loaded");

	ImGui::Text("Format: ");
	ImGui::SameLine();
	ImGui::TextColored(IMGUI_YELLOW, (res) ? res->GetFormatStr() : "???");
	ImGui::SameLine();
	ImGui::Checkbox("Is 2D", &component->is_2d);

	ImGui::SliderFloat("Fade In", (float*)&component->fade_in, 0.0f, 10.0f);
	ImGui::SliderFloat("Fade Out", (float*)&component->fade_out, 0.0f, 10.0f);
	ImGui::DragFloat("Min Distance", (float*)&component->min_distance, 0.1f, 0.1f, 10000.0f);
	ImGui::DragFloat("Max Distance", (float*)&component->max_distance, 0.1f, 0.1f, 10000.0f);
	ImGui::SliderInt("Cone In", (int*)&component->cone_angle_in, 0, 360);
	ImGui::SliderInt("Cone Out", (int*)&component->cone_angle_out, 0, 360);
	ImGui::SliderFloat("Vol Out Cone", (float*)&component->out_cone_vol, 0.0f, 1.0f);
	
	static const char * states[] = { 
		"Not Loaded", 
		"Stopped",
		"About to Play",
		"Playing",
		"About to Pause",
		"Pause",
		"About to Unpause",
		"About to Stop"
	};

	ImGui::Text("Current State: ");
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", states[component->GetCurrentState()]);

	if (ImGui::Button("Play"))
		component->Play();

	ImGui::SameLine();
	if (ImGui::Button("Pause"))
		component->Pause();

	ImGui::SameLine();
	if (ImGui::Button("Unpause"))
		component->UnPause();

	ImGui::SameLine();
	if (ImGui::Button("Stop"))
		component->Stop();
}

void PanelProperties::DrawAudioListenerComponent(ComponentAudioListener * component)
{
	ImGui::DragFloat("Distance", (float*)&component->distance, 0.1f, 0.1f, 10000.0f);
	ImGui::SliderFloat("Roll Off", (float*)&component->roll_off, 0.0f, 10.0f);
	ImGui::SliderFloat("Doppler", (float*)&component->doppler, 0.0f, 10.0f);
}

void PanelProperties::DrawCameraComponent(ComponentCamera * component)
{
	float near_plane = component->GetNearPlaneDist();
	if (ImGui::DragFloat("Near Plane", &near_plane, 0.1f, 0.1f, 1000.0f))
		component->SetNearPlaneDist(near_plane);

	float far_plane = component->GetFarPlaneDist();
	if (ImGui::DragFloat("Far Plane", &far_plane, 10.0f, 25.f, 10000.0f))
		component->SetFarPlaneDist(far_plane);

	float fov = component->GetFOV();
	if (ImGui::SliderFloat("Field of View", &fov, 1.0f, 179.0f))
		component->SetFOV(fov);

	float aspect_ratio = component->GetAspectRatio();
	if (ImGui::DragFloat("Aspect Ratio", &aspect_ratio, 0.1f, 0.1f, 10000.0f))
		component->SetAspectRatio(aspect_ratio);

	bool is_active = App->renderer3D->active_camera == component;
	if (ImGui::Checkbox("Is Active Camera", &is_active))
	{
		if(is_active == true)
			App->renderer3D->active_camera = component;
		else
			App->renderer3D->active_camera = App->camera->GetDummy();
	}

    bool is_culling = App->renderer3D->culling_camera == component;
    if (ImGui::Checkbox("Is Culling Camera", &is_culling))
    {
        if (is_culling == true)
            App->renderer3D->culling_camera = component;
        else
            App->renderer3D->culling_camera = App->camera->GetDummy();
    }
}

void PanelProperties::DrawMaterialResource(ResourceMaterial* material, ResourceMesh* mesh)
{
    bool modified = false;

    char tmp[256];
    strcpy_s(tmp, 255, material->GetUserResName());
    if(ImGui::InputText("Material name", tmp, 256))
    {
        material->SetUserResName(tmp);
        modified = true;
    }

    if (material->GetWorkFlow() == SpecularGlossiness)
    {
        SpecularGlossData sgData = material->GetSpecularGlossData();

        if (ImGui::CollapsingHeader("Ligthmap", ImGuiTreeNodeFlags_DefaultOpen))
        {
            modified = TextureButton(sgData.textures[SG_TextureLightmap], mesh, "Lightmap") || modified;
        }

        if (ImGui::CollapsingHeader("Ambient", ImGuiTreeNodeFlags_DefaultOpen))
        {
            modified = TextureButton(sgData.textures[SG_TextureOcclusion], mesh, "Occlusion") || modified;
        }

        if (ImGui::CollapsingHeader("Diffuse", ImGuiTreeNodeFlags_DefaultOpen))
        {
            modified = TextureButton(sgData.textures[SG_TextureDiffuse], mesh, "Diffuse") || modified;
            ImGui::PushID("diffuse");
            if (ImGui::ColorEdit4("color", (float*)&sgData.diffuse_color))
            {
                modified = true;
            }
            ImGui::PopID();
        }

        if (ImGui::CollapsingHeader("Specular", ImGuiTreeNodeFlags_DefaultOpen))
        {
            modified = TextureButton(sgData.textures[SG_TextureSpecular], mesh, "Specular") || modified;
            ImGui::PushID("specular");
            if (ImGui::ColorEdit3("color", (float*)&sgData.specular_color))
            {
                modified = true;
            }

            if (ImGui::SliderFloat("Intensity", &sgData.specular_intensity, 1.0f, 50.0f))
            {
                modified = true;
            }

            ImGui::PopID();

            if (ImGui::SliderFloat("Smoothness", &sgData.smoothness, 0.0f, 10.0f))
            {
                modified = true;
            }

            bool planar = material->GetPlanarReflections();
            if (ImGui::Checkbox("Use planar reflections", &planar))
            {
                material->SetPlanarReflections(planar);
                modified = true;
            }
        }

        if (ImGui::CollapsingHeader("Normal", ImGuiTreeNodeFlags_DefaultOpen))
        {
            modified = TextureButton(sgData.textures[SG_TextureNormal], mesh, "Normal") || modified;

            if (ImGui::SliderFloat("Strength", &sgData.normal_strength, 0.0f, 10.0f))
            {
                modified = true;
            }
        }

        if (ImGui::CollapsingHeader("Emissive", ImGuiTreeNodeFlags_DefaultOpen))
        {
            modified = TextureButton(sgData.textures[SG_TextureEmissive], mesh, "Emissive") || modified;
            ImGui::PushID("emissive");
            if (ImGui::ColorEdit3("color", (float*)&sgData.emissive_color))
            {
                modified = true;
            }

            ImGui::PopID();

            if (ImGui::SliderFloat("Intensity", &sgData.emissive_intensity, 1.0f, 50.0f))
            {
                modified = true;
            }
        }

        if (ImGui::CollapsingHeader("Detail Mask", ImGuiTreeNodeFlags_DefaultOpen))
        {
            modified = TextureButton(sgData.textures[SG_TextureDetailMask], mesh, "Detail Mask") || modified;
        }

        float2 tiling = material->GetUVTiling();
        if (ImGui::DragFloat2("Tiling", &tiling.x, 0.1f, 0.1f, 1000.0f))
        {
            material->SetUVTiling(tiling);
            modified = true;
        }

        float2 offset = material->GetUVOffset();
        if (ImGui::DragFloat2("Offset", &offset.x, 0.01f, 0.0f, 1.0f))
        {
            material->SetUVOffset(offset);
            modified = true;
        }

        if (ImGui::CollapsingHeader("Secondary Diffuse", ImGuiTreeNodeFlags_DefaultOpen))
        {
            modified = TextureButton(sgData.textures[SG_TextureScndDiffuse], mesh, "Secondary Diffuse") || modified;
        }

        if (ImGui::CollapsingHeader("Secondary Specular", ImGuiTreeNodeFlags_DefaultOpen))
        {
            modified = TextureButton(sgData.textures[SG_TextureScndDiffuse], mesh, "Secondary Specular") || modified;
        }

        if (ImGui::CollapsingHeader("Secondary Normal", ImGuiTreeNodeFlags_DefaultOpen))
        {
            modified = TextureButton(sgData.textures[SG_TextureScndDiffuse], mesh, "Secondary Normal") || modified;
        }

        tiling = material->GetSecondUVTiling();
        if (ImGui::DragFloat2("Secondary Tiling", &tiling.x, 0.1f, 0.1f, 1000.0f))
        {
            material->SetSecondUVTiling(tiling);
            modified = true;
        }

        offset = material->GetSecondUVOffset();
        if (ImGui::DragFloat2("Secondary Offset", &offset.x, 0.01f, 0.0f, 1.0f))
        {
            material->SetSecondUVOffset(offset);
            modified = true;
        }

        bool dsided = material->GetDoubleSided();

        if (ImGui::Checkbox("Double side", &dsided))
        {
            material->SetDoubleSided(dsided);
            modified = true;
        }

        float atest = material->GetAlphaTest();

        if (ImGui::SliderFloat("Alpha test", &atest, 0.0f, 1.0f))
        {
            material->SetAlphaTest(atest);
            modified = true;
        }

        if (modified)
        {
            material->SetSpecularGlossData(sgData);
        }
    }
    else
    {
        MetallicRoughData mrData = material->GetMetallicRoughData();

        if (ImGui::CollapsingHeader("Base Colour", ImGuiTreeNodeFlags_DefaultOpen))
        {
            modified = TextureButton(mrData.textures[MR_TextureBaseColor], mesh, "Base Colour") || modified;
            ImGui::PushID("Base Colour");
            if (ImGui::ColorEdit4("color", (float*)&mrData.baseColor))
            {
                modified = true;
            }
            ImGui::PopID();
        }

        if (ImGui::CollapsingHeader("MetallicRoughness", ImGuiTreeNodeFlags_DefaultOpen))
        {
            modified = TextureButton(mrData.textures[MR_TextureMetallicRough], mesh, "MetallicRoughness") || modified;
            ImGui::PushID("MetallilcRoughness");
            if (ImGui::SliderFloat("Metallic", &mrData.metalness, 0.0f, 1.0f))
            {
                modified = true;
            }

            if (ImGui::SliderFloat("Roughness", &mrData.roughness, 0.0f, 1.0f))
            {
                modified = true;
            }

            ImGui::PopID();

            bool planar = material->GetPlanarReflections();
            if (ImGui::Checkbox("Use planar reflections", &planar))
            {
                material->SetPlanarReflections(planar);
                modified = true;
            }
        }

        if (ImGui::CollapsingHeader("Normal", ImGuiTreeNodeFlags_DefaultOpen))
        {
            modified = TextureButton(mrData.textures[MR_TextureNormal], mesh, "Normal") || modified;

            if (ImGui::SliderFloat("Normal Strength", &mrData.normal_strength, 0.0f, 10.0f))
            {
                modified = true;
            }
        }

        if (ImGui::CollapsingHeader("Emissive", ImGuiTreeNodeFlags_DefaultOpen))
        {
            modified = TextureButton(mrData.textures[MR_TextureEmissive], mesh, "Emissive") || modified;
            ImGui::PushID("emissive");
            if (ImGui::ColorEdit3("color", (float*)&mrData.emissive_color))
            {
                modified = true;
            }

            ImGui::PopID();

            if (ImGui::SliderFloat("Intensity", &mrData.emissive_intensity, 1.0f, 50.0f))
            {
                modified = true;
            }
        }

        if (ImGui::CollapsingHeader("Occlusion", ImGuiTreeNodeFlags_DefaultOpen))
        {
            modified = TextureButton(mrData.textures[MR_TextureOcclusion], mesh, "Occlusion") || modified;
            if (ImGui::SliderFloat("Occlusion Strength", &mrData.occlusion_strength, 0.0f, 50.0f))
            {
                modified = true;
            }
        }

        if (modified)
        {
            material->SetMetallicRoughData(mrData);
        }
    }


    /**/

    if(modified)
    {
        material->Save();
        App->renderer->GetBatchManager()->OnMaterialModified(material->GetUID());
    }

}

UID PanelProperties::TextureButton(ResourceTexture* texture, ResourceMesh* mesh, const char* name, int uniqueId, bool& modified)
{
    UID res = texture ? texture->GetUID() : 0;

    ImVec2 size(64.0f, 64.0f);

    if(texture != nullptr)
    {
		ImGui::PushID(name);
        if(texture->GetMetadata().texType == TextureType_2D && ImGui::ImageButton(ImTextureID(size_t(texture->GetID())), size, ImVec2(0,1), ImVec2(1,0), ImColor(255, 255, 255, 128), ImColor(255, 255, 255, 128)))
        {
			ImGui::PopID();

            show_texture.Open(mesh, texture);
        }
        else 
        {
			ImGui::PopID();

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", texture->GetFile());
			}
        }

        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Text("Texture:");
        ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_YELLOW);

        std::string file;
        App->fs->SplitFilePath(texture->GetFile(), nullptr, &file);
#if 0
        ImGui::Text("%s", file.c_str());
        ImGui::Text("(%u,%u) %s (%u Kb) %s", texture->GetWidth(), texture->GetHeight(), texture->GetFormatStr(), texture->GetMemSize() >> 10, texture->IsCompressed() ? "compressed" : "");
#endif 
        ImGui::PopStyleColor();

        ImGui::SameLine();
        char tmp[128];
        sprintf_s(tmp, 127, "%sLinear", name);
        ImGui::PushID(tmp);
        bool gamma = texture->GetColorSpace() == ColorSpace_gamma;
        if(ImGui::Checkbox("sRGB", &gamma))
        {
            modified = true;
            texture->SetColorSpace(gamma ? ColorSpace_gamma : ColorSpace_linear);
            texture->ReleaseFromMemory();
            texture->LoadInMemory();
        }
        ImGui::PopID();

        ImGui::PushID(name);
        if(ImGui::SmallButton("Delete"))
        {
            res = 0;
        }
        ImGui::PopID();

        ImGui::SameLine();
        ImGui::PushID(name);
        if(ImGui::SmallButton("Select texture"))
        {
            ImGui::PopID();
            selectTexture.Open(Resource::texture, name, uniqueId);
        }
        else
        {
            ImGui::PopID();
        }

        ImGui::EndGroup();
    }
    else
    {
        ImGui::PushID(name);
        if(ImGui::SmallButton("Select texture"))
        {
            ImGui::PopID();
            selectTexture.Open(Resource::texture, name, uniqueId);
        }
        else
        {
            ImGui::PopID();
        }
    }

    if(selectTexture.HasSelection(uniqueId))
    {
        res = selectTexture.GetResource();
        selectTexture.ClearSelection();
    }

    return res;
}

bool PanelProperties::TextureButton(UID& uid, ResourceMesh* mesh, const char* name)
{
    bool modified = false;
    ResourceTexture* tex = static_cast<ResourceTexture*>(App->resources->Get(uid));
    UID newUID = TextureButton(tex, mesh, name, int(uid), modified);
    modified = modified || newUID != uid;
    uid = newUID;

    if (modified)
    {
        int i = 0;
        ++i;
    }

    return modified;
}

void PanelProperties::DrawAnimationComponent(ComponentAnimation* component)
{
    ResourceStateMachine* state_res = component->GetResource();

    UID new_res = PickResourceModal(Resource::state_machine);

    ImGui::SameLine();
    if(ImGui::Button("New State machine"))
    {
        ResourceStateMachine* state_machine = static_cast<ResourceStateMachine*>(App->resources->CreateNewResource(Resource::state_machine, 0));
        App->resources->SaveResources();

        bool save_ok = state_machine->Save();

        if(save_ok)
        {
            new_res = state_machine->GetUID();
        }
    }


    bool debug_draw = component->GetDebugDraw();
    if(ImGui::Checkbox("Debug draw", &debug_draw))
    {
        component->SetDebugDraw(debug_draw);
    }

    if(new_res > 0)
    {
		component->SetResource(new_res);
    }
    else if(state_res != nullptr)
    {
        char name[128];

        strcpy_s(name, state_res->GetUserResName());

        if(ImGui::InputText("Resource name", name, 128))
        {
            state_res->SetName(name);
            state_res->Save();
        }

        if (ImGui::Button("Add clip"))
        {
            state_res->AddClip(HashString("noname"), 0, true);
            state_res->Save();
        }

        ImGui::Separator();

        uint i=0; 
        while(i < state_res->GetNumClips())
        {
            ResourceAnimation* resource = static_cast<ResourceAnimation*>(App->resources->Get(state_res->GetClipRes(i)));

            strcpy_s(name, state_res->GetClipName(i).C_str());

            ImGui::PushID(i);
            if(ImGui::InputText("Clip name", name, 128))
            {
                state_res->SetClipName(i, HashString(name));
                state_res->Save();
            }

            ImGui::LabelText("Resource", resource ? resource->GetUserResName() : "Unknown");
            ImGui::SameLine();
            if(ImGui::ArrowButton("resource", ImGuiDir_Right))
            {
                ImGui::OpenPopup("Select");
            }

            UID new_res = OpenResourceModal(Resource::animation, "Select");


            if (new_res > 0)
            {
                state_res->SetClipRes(i, new_res);
                state_res->Save();

                App->resources->Get(new_res)->LoadToMemory();
            }

            bool loop = state_res->GetClipLoop(i);
            if(ImGui::Checkbox("Loop", &loop))
            {
                state_res->SetClipLoop(i, loop);
                state_res->Save();
            }

            ImGui::SameLine();
            if(ImGui::Button("Remove"))
            {
                state_res->RemoveClip(i);
                state_res->Save();
            }
            else
            {
                ++i;
            }

            ImGui::Separator();
            ImGui::PopID();
        }

        if(App->GetState() != Application::stop)
        {

            if (ImGui::CollapsingHeader("Animation triggers", ImGuiTreeNodeFlags_DefaultOpen))
            {
                HashString active_node = component->GetActiveNode();

                for(uint i=0; i< state_res->GetNumTransitions(); ++i)
                {
                    if(state_res->GetTransitionSource(i) == active_node)
                    {
                        HashString trigger = state_res->GetTransitionTrigger(i);
                        if(trigger && ImGui::Button(trigger.C_str()))
                        {
                            component->SendTrigger(trigger);
                        }
                    }
                }

                if(ImGui::Button("Reset state"))
                {
                    component->ResetState();
                }
            }
        }
    }
}

void PanelProperties::DrawRootMotionComponent(ComponentRootMotion * component)
{
}

void PanelProperties::DrawMesh(const ResourceMesh* res)
{
    if (res != nullptr)
    {
        ImGui::TextColored(IMGUI_YELLOW, "%s", res->GetFile());
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("Type: %s", res->GetTypeStr());
            ImGui::Text("UID: %llu", res->GetUID());
            ImGui::Text("Lib: %s", res->GetExportedFile());
            ImGui::EndTooltip();
        }

        ImGui::TextColored(ImVec4(1, 1, 0, 1), "%u Triangles (%u indices %u vertices)", res->num_indices / 3, res->num_indices, res->num_vertices);

        char attributes[256];
        strcpy_s(attributes, "\nAttributes: \n\n\tPositions");
        if (res->HasAttrib(ATTRIB_TEX_COORDS_0))
        {
            strcat_s(attributes, "\n\tTexCoords0");
        }
        if (res->HasAttrib(ATTRIB_TEX_COORDS_1))
        {
            strcat_s(attributes, "\n\tTexCoords1");
        }
        if (res->HasAttrib(ATTRIB_NORMALS))
        {
            strcat_s(attributes, "\n\tNormals");
        }
        if (res->HasAttrib(ATTRIB_TANGENTS))
        {
            strcat_s(attributes, "\n\tTangents");
        }
        if (res->HasAttrib(ATTRIB_BONE_INDICES))
        {
            strcat_s(attributes, "\n\tBones");
        }
        strcat_s(attributes, "\n\n");

        ImGui::TextColored(ImVec4(1, 1, 0, 1), attributes);
    }
}

void PanelProperties::DrawSpotConeComponent(ComponentSpotCone *spotCone)
{
    float height = spotCone->getHeight();
    if(ImGui::DragFloat("Height", &height, 0.01f, 0.0f, 1000.0f))
    {
        spotCone->setHeight(height);
    }

    float radius = spotCone->getRadius();
    if(ImGui::DragFloat("Radius", &radius, 0.01f, 0.0f, 1000.0f))
    {
        spotCone->setRadius(radius);
    }

    float4 colour = spotCone->getColour();
    if(ImGui::ColorEdit3("Colour", &colour[0]))
    {
        spotCone->setColour(colour);
    }

    if(ImGui::DragFloat("Intensity", &colour[3]))
    {
        spotCone->setColour(colour);
    }

    float transparency = spotCone->getTransparency();
    if(ImGui::DragFloat("Transparency", &transparency, 0.01f, 0.0f, 1.0f))
    {
        spotCone->setTransparency(transparency);
    }

    float smoothAmount = spotCone->getSmoothAmount();
    if(ImGui::DragFloat("Smooth amount", &smoothAmount, 0.1f, 0.0, 100.0f))
    {
        spotCone->setSmoothAmount(smoothAmount);
    }

    float fresnelAmount = spotCone->getFresnelAmount();
    if(ImGui::DragFloat("Fresnel amount", &fresnelAmount, 0.1f, 0.0f, 100.0f))
    {
        spotCone->setFresnelAmount(fresnelAmount);
    }

    bool modified = false;

    if (ImGui::CollapsingHeader("Fog0", ImGuiTreeNodeFlags_DefaultOpen))
    {
        UID newUID = TextureButton(spotCone->getFog0Res(), nullptr, "Fog0", 0, modified);
        if (newUID != spotCone->getFog0())
        {
            spotCone->setFog0(newUID);
        }

        float2 tiling = spotCone->getFog0Tiling();
        if(ImGui::InputFloat2("Tiling", &tiling[0]))
        {
            spotCone->setFog0Tiling(tiling);
        }

        float2 offset = spotCone->getFog0Offset();
        if(ImGui::InputFloat2("Offset", &offset[0]))
        {
            spotCone->setFog0Offset(offset);
        }

        float2 speed = spotCone->getFog0Speed();
        if(ImGui::InputFloat2("Speed", &speed[0]))
        {
            spotCone->setFog0Speed(speed);
        }
    }

    if (ImGui::CollapsingHeader("Fog1", ImGuiTreeNodeFlags_DefaultOpen))
    {
        UID newUID = TextureButton(spotCone->getFog1Res(), nullptr, "Fog1", 1, modified);
        if (newUID != spotCone->getFog1())
        {
            spotCone->setFog1(newUID);
        }

        float2 tiling = spotCone->getFog1Tiling();
        ImGui::PushID("Tiling1");
        if(ImGui::InputFloat2("Tiling", &tiling[0]))
        {
            spotCone->setFog1Tiling(tiling);
        }
        ImGui::PopID();

        float2 offset = spotCone->getFog1Offset();
        ImGui::PushID("Offset1");
        if(ImGui::InputFloat2("Offset", &offset[0]))
        {
            spotCone->setFog1Offset(offset);
        }
        ImGui::PopID();

        float2 speed = spotCone->getFog1Speed();
        ImGui::PushID("Speed1");
        if(ImGui::InputFloat2("Speed", &speed[0]))
        {
            spotCone->setFog1Speed(speed);
        }
        ImGui::PopID();
    }
}

void PanelProperties::DrawDecalComponent(ComponentDecal* decal)
{
    bool modified = false;

    if (ImGui::CollapsingHeader("Albedo", ImGuiTreeNodeFlags_DefaultOpen))
    {
        UID newUID = TextureButton(decal->GetAlbedoRes(), nullptr, "Albedo", 0, modified);
        if (newUID != decal->GetAlbedo())
        {
            decal->SetAlbedo(newUID);
            modified = true;
        }
    }

    if (ImGui::CollapsingHeader("Normal", ImGuiTreeNodeFlags_DefaultOpen))
    {
        UID newUID = TextureButton(decal->GetNormalRes(), nullptr, "Normal", 1, modified);
        if(newUID != decal->GetNormal())
        {
            decal->SetNormal(newUID);
            modified = true;
        }

        float strength = Clamp(decal->GetNormalStrength(), 0.0f, 100.0f);
        if(ImGui::SliderFloat("Strength", &strength, 0.0f, 10.0f))
        {
            decal->SetNormalStrength(strength);
            modified = true;
        }
    }

    if (ImGui::CollapsingHeader("Specular", ImGuiTreeNodeFlags_DefaultOpen))
    {
        UID newUID = TextureButton(decal->GetSpecularRes(), nullptr, "Specular", 2, modified);
        if(newUID != decal->GetSpecular())
        {
            decal->SetSpecular(newUID);
            modified = true;
        }
    }


}

void PanelProperties::DrawGrassComponent(ComponentGrass* component)
{
	ResourceMesh* mesh_res = component->GetMesh();

    DrawMesh(mesh_res);

    UID new_res = PickResourceModal(Resource::mesh);

    if (new_res > 0)
    {
        component->SetMesh(new_res);
    }

    ResourceMaterial* mat_res = component->GetMaterial();

    new_res = PickResourceModal(Resource::material);

    ImGui::SameLine();
    if(ImGui::Button("New material"))
    {
        ResourceMaterial* material = static_cast<ResourceMaterial*>(App->resources->CreateNewResource(Resource::material, 0));

        bool save_ok = material->Save();

        if(save_ok)
        {
            new_res = material->GetUID();
        }
    }

    ImGui::Separator();


    if(new_res > 0)
    {
		component->SetMaterial(new_res);
    }
    else if(mat_res)
	{
		DrawMaterialResource(mat_res, mesh_res);
    }

    static const int noise_count = 256;

    float (*random_getter)(void* data, int idx) = [](void* data, int idx) -> float 
    { 
        float scale = 25.0f;
        return ((ComponentGrass*)data)->RandomNoise(idx);
    };

    float (*values_getter)(void* data, int idx) = [](void* data, int idx) -> float 
    { 
        float scale = 25.0f;
        return ((ComponentGrass*)data)->ValueNoise(scale*float(idx)/float(noise_count)); 
    };

    ImGui::PlotLines("Random Noise", random_getter, component, noise_count, 0, nullptr, 0.0f, 1.0f, ImVec2((float)noise_count, 200.0f));
    ImGui::PlotLines("Value Noise", values_getter, component, noise_count, 0, nullptr, 0.0f, 1.0f, ImVec2((float)noise_count, 200.0f));
}

void DrawLineComponent(ComponentLine* component)
{
    bool modified = false;
    ResourceTexture* info = component->GetTexture().GetPtr<ResourceTexture>();

    if(ImGui::CollapsingHeader("Texture", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImVec2 size(64.0f, 64.0f);

        if (info != nullptr)
        {
            if(ImGui::ImageButton((ImTextureID) (ImTextureID)size_t(info->GetID()), size, ImVec2(0,1), ImVec2(1,0), ImColor(255, 255, 255, 128), ImColor(255, 255, 255, 128)))
            {
                ImGui::OpenPopup("texture");
            }
            else 
            {
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("%s", info->GetFile());
                }
            }
        }
        else
        {
            if(ImGui::ImageButton((ImTextureID) 0, size, ImVec2(0,1), ImVec2(1,0), ImColor(255, 255, 255, 128)))
            {
                ImGui::OpenPopup("texture");
            }
        }

        ImGui::SameLine();
        ImGui::BeginGroup();
        if(info != nullptr)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_YELLOW);

            std::string file;
            App->fs->SplitFilePath(info->GetFile(), nullptr, &file);
#if 0
            ImGui::Text("%s", file.c_str());
            ImGui::Text("(%u,%u) %s %s", info->GetWidth(), info->GetHeight(), info->GetFormatStr(), info->IsCompressed() ? "compressed" : "");
#endif
            ImGui::PopStyleColor();

            ImGui::SameLine();
            bool linear = info->GetColorSpace() == ColorSpace_linear;
            if(ImGui::Checkbox("sRGB", &linear))
            {
                info->SetColorSpace(linear ? ColorSpace_linear : ColorSpace_gamma);
                info->ReleaseFromMemory();
                info->LoadInMemory();
            }

            if(ImGui::SmallButton("Delete"))
            {
                component->SetTexture(0);
            }
        }
        ImGui::EndGroup();

        UID new_res = App->editor->props->OpenResourceModal(Resource::texture, "texture");

        if(new_res != 0)
        {
            component->SetTexture(new_res);
        }
    }

    float speed = component->GetSpeedMult();
    if(ImGui::DragFloat("Speed", &speed, 0.01f, 0.0f, 50.0f))
    {
        component->SetSpeedMult(speed);
    }

    float fadeInTime = component->GetFadeInTime();
    if(ImGui::DragFloat("Fade In Time", &fadeInTime, 0.01f, 0.0f, 10.0f))
    {
        component->SetFadeInTime(fadeInTime);
    }

    float fadeOutTime = component->GetFadeOutTime();
    if(ImGui::DragFloat("Fade Out Time", &fadeOutTime, 0.01f, 0.0f, 10.0f))
    {
        component->SetFadeOutTime(fadeOutTime);
    }

    float2 tiling = component->GetTiling();
    if(ImGui::DragFloat2("Tiling", &tiling.x, 0.1f, 0.0f, 50.0f))
    {
        component->SetTiling(tiling);
    }

    float2 offset = component->GetOffset();
    if(ImGui::DragFloat2("Offset", &offset.x, 0.1f, 0.0f, 50.0f))
    {
        component->SetOffset(offset);
    }

    int numBillboards = int(component->GetNumBillboards());
    if(ImGui::SliderInt("Num billboards", &numBillboards, 1, 500))
    {
        component->SetNumBillboards(uint(numBillboards));
    }

    if (ImGui::CollapsingHeader("Size", ImGuiTreeNodeFlags_DefaultOpen))
    {
        float4 sizePoints = component->GetSizeOverTimePoints();

        if (ImGui::Bezier("Size", (float*)&sizePoints.x))
        {
            component->SetSizeOverTimePoints(sizePoints);
        }

        if (ImGui::Button("EaseIn", ImVec2(55, 20))) component->SetSizeOverTimePoints(float4(0.0f, 0.0f, 1.0f, 0.0f));
        ImGui::SameLine();
        if (ImGui::Button("EaseOut", ImVec2(60, 20))) component->SetSizeOverTimePoints(float4(0.0f, 0.0f, 0.0f, 1.f));
        ImGui::SameLine();
        if (ImGui::Button("EaseInOut", ImVec2(70, 20))) component->SetSizeOverTimePoints(float4(0.0, 1.0f, 1.0f, 0.0f));

        float2 sizeRange = component->GetSizeOverTimeRange();
        if (ImGui::DragFloat("init", &sizeRange.x, 0.01f, 0.0f, 100.0f)) component->SetSizeOverTimeRange(sizeRange);
        if (ImGui::DragFloat("end", &sizeRange.y, 0.01f, 0.0f, 100.0f)) component->SetSizeOverTimeRange(sizeRange);
    }

    
    if (ImGui::CollapsingHeader("Color", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::GradientButton(&component->GetColorGradient().gradient))
        {
            ImGui::OpenPopup("Show color gradient");
        }

        if (ImGui::BeginPopup("Show color gradient"))
        {
            ImGui::GradientEditor(&component->GetColorGradient().gradient, component->GetColorGradient().draggingMark, component->GetColorGradient().selectedMark);
            ImGui::EndPopup();
        }
    }

    if(ImGui::Button("Start"))
        component->Start();

    if(ImGui::Button("Stop"))
        component->Stop();

}

void DrawTrailComponent(ComponentTrail* component)
{
    bool modified = false;
    ResourceTexture* info = component->GetTextureRes();

    if(ImGui::CollapsingHeader("Texture", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (info)
        {
            char name[128];
            strcpy_s(name, info->GetUserResName());

            if (ImGui::InputText("Resource name", name, 128))
            {
                info->SetName(name);
            }
        }

        ImVec2 size(64.0f, 64.0f);

        if (info != nullptr)
        {
            if(ImGui::ImageButton((ImTextureID) (ImTextureID)size_t(info->GetID()), size, ImVec2(0,1), ImVec2(1,0), ImColor(255, 255, 255, 128), ImColor(255, 255, 255, 128)))
            {
                ImGui::OpenPopup("texture");
            }
            else 
            {
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("%s", info->GetFile());
                }
            }
        }
        else
        {
            if(ImGui::ImageButton((ImTextureID) 0, size, ImVec2(0,1), ImVec2(1,0), ImColor(255, 255, 255, 128)))
            {
                ImGui::OpenPopup("texture");
            }
        }

        ImGui::SameLine();
        ImGui::BeginGroup();
        if(info != nullptr)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_YELLOW);

            std::string file;
            App->fs->SplitFilePath(info->GetFile(), nullptr, &file);

#if 0
            ImGui::Text("%s", file.c_str());
            ImGui::Text("(%u,%u) %s %s", info->GetWidth(), info->GetHeight(), info->GetFormatStr(), info->IsCompressed() ? "compressed" : "");
#endif 
            ImGui::PopStyleColor();

            ImGui::SameLine();
            bool linear = info->GetColorSpace() == ColorSpace_linear;
            if(ImGui::Checkbox("sRGB", &linear))
            {
                info->SetColorSpace(linear ? ColorSpace_linear : ColorSpace_gamma);
            }

            if(ImGui::SmallButton("Delete"))
            {
                component->SetTexture(0);
            }
        }
        ImGui::EndGroup();

        UID new_res = App->editor->props->OpenResourceModal(Resource::texture, "texture");

        if(new_res != 0)
        {
            component->SetTexture(new_res);
        }
    }

    ImGui::InputFloat("Duration", &component->config_trail.duration, 0.1f);
    ImGui::InputFloat("Min vertex distance", &component->config_trail.min_vertex_distance, 0.1f);
    ImGui::InputFloat("Width", &component->config_trail.width, 0.1f);
    ImGui::SliderAngle("Min Angle to add vretices", &component->config_trail.MinAngleToAddVertices, 0.0f);
    ImGui::InputInt("Num add vertices", (int*)&component->config_trail.NumAddVertices);
    ImGui::Checkbox("Linear", &component->config_trail.linear);

    if(ImGui::GradientButton(&component->color_over_time.gradient))
    {
        ImGui::OpenPopup("Show color gradient");
    }

    if (ImGui::BeginPopup("Show color gradient"))
    {
        bool updated = ImGui::GradientEditor(&component->color_over_time.gradient, component->color_over_time.draggingMark, 
                component->color_over_time.selectedMark);
        ImGui::EndPopup();
    }

    ImGui::Bezier("Size", (float*)&component->size_over_time.bezier);

    if(ImGui::Button("EaseIn", ImVec2(55, 20))) component->size_over_time.bezier = float4(0.0f, 0.0f, 1.0f, 0.0f);
    ImGui::SameLine();
    if(ImGui::Button("EaseOut", ImVec2(60, 20))) component->size_over_time.bezier = float4(0.0f, 0.0f, 0.0f, 1.f);
    ImGui::SameLine();
    if(ImGui::Button("EaseInOut", ImVec2(70, 20))) component->size_over_time.bezier = float4(0.0, 1.0f, 1.0f, 0.0f);


        ImGui::DragFloat("init", &component->size_over_time.init);
        ImGui::DragFloat("end", &component->size_over_time.end);

    const char* names[ComponentTrail::BlendCount] = { "Additive", "Alpha" };
    ImGui::Combo("Blend mode", (int*)&component->blend_mode, names, int(ComponentTrail::BlendCount));

    const char* texture_names[ComponentTrail::TextureCount] = { "Stretch", "Wrap" };
    ImGui::Combo("Texture mode", (int*)&component->texture_mode, texture_names, int(ComponentTrail::TextureCount));
}

void PanelProperties::DrawParticleSystemComponent(ComponentParticleSystem* component)
{
    bool modified = false;
    ResourceTexture* info = component->GetTextureRes();

    if(ImGui::CollapsingHeader("Texture", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (info)
        {
            char name[128];
            strcpy_s(name, info->GetUserResName());

            if (ImGui::InputText("Resource name", name, 128))
            {
                info->SetName(name);
            }
        }

        ImVec2 size(64.0f, 64.0f);

        if (info != nullptr)
        {
            if(ImGui::ImageButton((ImTextureID) size_t(info->GetID()), size, ImVec2(0,1), ImVec2(1,0), ImColor(255, 255, 255, 128), ImColor(255, 255, 255, 128)))
            {
                ImGui::OpenPopup("texture");
            }
            else 
            {
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("%s", info->GetFile());
                }
            }
        }
        else
        {
            if(ImGui::ImageButton((ImTextureID) 0, size, ImVec2(0,1), ImVec2(1,0), ImColor(255, 255, 255, 128)))
            {
                ImGui::OpenPopup("texture");
            }
        }

        ImGui::SameLine();
        ImGui::BeginGroup();
        if(info != nullptr)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_YELLOW);

            std::string file;
            App->fs->SplitFilePath(info->GetFile(), nullptr, &file);
#if 0
            ImGui::Text("%s", file.c_str());
            ImGui::Text("(%u,%u) %s %s", info->GetWidth(), info->GetHeight(), info->GetFormatStr(), info->IsCompressed() ? "compressed" : "");
#endif 
            ImGui::PopStyleColor();

            ImGui::SameLine();
            bool linear = info->GetColorSpace() != ColorSpace_linear;
            if(ImGui::Checkbox("sRGB", &linear))
            {
                info->SetColorSpace(linear ? ColorSpace_gamma : ColorSpace_linear);
            }

            if(ImGui::SmallButton("Delete"))
            {
                component->SetTexture(0);
            }
        }
        ImGui::EndGroup();

        UID new_res = App->editor->props->OpenResourceModal(Resource::texture, "texture");

        if(new_res != 0)
        {
            component->SetTexture(new_res);
        }

        ImGui::InputInt("sheet x tiles", (int*)&component->texture_info.x_tiles); 
        ImGui::InputInt("sheet y tiles", (int*)&component->texture_info.y_tiles);

        ImGui::Checkbox("Visible" , &component->visible);
    }

    if(ImGui::CollapsingHeader("Initialization", ImGuiTreeNodeFlags_DefaultOpen))
    {
        auto ShowRandom = [](const char* name, ComponentParticleSystem::RandomValue* value, bool angle)
        {
            if(angle)
            {
                value->range[0] *= 180.0f/PI;
                value->range[1] *= 180.0f/PI;
            }

            if(value->random)
            {
                ImGui::InputFloat2(name, value->range);
            }
            else
            {
                ImGui::InputFloat(name, &value->range[0], 0.001f);
            }

            if(angle)
            {
                value->range[0] *= PI/180.0f;
                value->range[1] *= PI/180.0f;
            }

            ImGui::SameLine();
            ImGui::PushID(name);
            ImGui::Checkbox("Ran", &value->random);
            ImGui::PopID();
        };

        ImGui::InputInt("Max particles", (int*)&component->init.max_particles);
        ImGui::Checkbox("Loop", &component->init.loop);
        ImGui::InputFloat("Duration", &component->init.duration, 0.01f);
        ShowRandom("Life ", &component->init.life, false);
        ShowRandom("Speed", &component->init.speed, false);
        ShowRandom("Size ", &component->init.size, false);
        ShowRandom("Rot  ", &component->init.rotation, true);
        ShowRandom("Grav ", &component->init.gravity, false);
        ImGui::ColorEdit4("Color", (float*)&component->init.color);
        ImGui::InputFloat("Whole speed", &component->init.whole_speed, 0.01f);
    }

    if(ImGui::CollapsingHeader("Emitter", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::InputInt("Particles per second", (int*)&component->emitter.particles_per_second);

        const char* names[ComponentParticleSystem::ShapeCount] = { "Circle", "Cone" };

        ImGui::Combo("Type", (int*)&component->shape.type, names, int(ComponentParticleSystem::ShapeCount));
        if(component->shape.type == ComponentParticleSystem::Circle)
        {
            ImGui::InputFloat("Radius", &component->shape.radius, 0.01f);
        }
        else if(component->shape.type == ComponentParticleSystem::Cone)
        {
            ImGui::SliderAngle("Angle", &component->shape.angle, 0.0, 90.0f);
            ImGui::InputFloat("Radius", &component->shape.radius, 0.01f);
        }
    }

    if(ImGui::CollapsingHeader("Speed over time", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID("SpeedOverTime");
        ImGui::Bezier("Speed", (float*)&component->speed_over_time.bezier);
        ImGui::PopID();

        if(ImGui::Button("EaseIn", ImVec2(55, 20))) component->speed_over_time.bezier = float4(0.0f, 0.0f, 1.0f, 0.0f);
        ImGui::SameLine();
        if(ImGui::Button("EaseOut", ImVec2(60, 20))) component->speed_over_time.bezier = float4(0.0f, 0.0f, 0.0f, 1.f);
        ImGui::SameLine();
        if(ImGui::Button("EaseInOut", ImVec2(70, 20))) component->speed_over_time.bezier = float4(0.0, 1.0f, 1.0f, 0.0f);

        ImGui::DragFloat3("init", (float*)&component->speed_over_time.init);
        ImGui::DragFloat3("end", (float*)&component->speed_over_time.end);

    }

    ImGui::PushID("Size");
    if(ImGui::CollapsingHeader("Size over time", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Bezier("Size", (float*)&component->size_over_time.bezier);

        if(ImGui::Button("EaseIn", ImVec2(55, 20))) component->size_over_time.bezier = float4(0.0f, 0.0f, 1.0f, 0.0f);
        ImGui::SameLine();
        if(ImGui::Button("EaseOut", ImVec2(60, 20))) component->size_over_time.bezier = float4(0.0f, 0.0f, 0.0f, 1.f);
        ImGui::SameLine();
        if(ImGui::Button("EaseInOut", ImVec2(70, 20))) component->size_over_time.bezier = float4(0.0, 1.0f, 1.0f, 0.0f);


        ImGui::DragFloat("init", &component->size_over_time.init);
        ImGui::DragFloat("end", &component->size_over_time.end);
    }
    ImGui::PopID();

    perlin->Draw(component->noise_params);

    ImGui::PushID("Frame");
    if(ImGui::CollapsingHeader("Frame over time", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("random", &component->texture_info.random);

        if(!component->texture_info.random)
        {
            ImGui::Bezier("sheet frame over time", (float*)&component->texture_info.frame_over_time.bezier);

            if(ImGui::Button("EaseIn", ImVec2(55, 20))) component->texture_info.frame_over_time.bezier = float4(0.0f, 0.0f, 1.0f, 0.0f);
            ImGui::SameLine();
            if(ImGui::Button("EaseOut", ImVec2(60, 20))) component->texture_info.frame_over_time.bezier = float4(0.0f, 0.0f, 0.0f, 1.f);
            ImGui::SameLine();
            if(ImGui::Button("EaseInOut", ImVec2(70, 20))) component->texture_info.frame_over_time.bezier = float4(0.0, 1.0f, 1.0f, 0.0f);
        }

        ImGui::DragFloat("init", (float*)&component->texture_info.frame_over_time.init);
        ImGui::DragFloat("end", (float*)&component->texture_info.frame_over_time.end);
    }
    ImGui::PopID();

    if(ImGui::CollapsingHeader("Color over time", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if(ImGui::GradientButton(&component->color_over_time.gradient))
        {
			ImGui::OpenPopup("Show color gradient");
        }
 
        if (ImGui::BeginPopup("Show color gradient"))
        {
            bool updated = ImGui::GradientEditor(&component->color_over_time.gradient, component->color_over_time.draggingMark, 
                                                 component->color_over_time.selectedMark);
            ImGui::EndPopup();
        }
    }

    if(ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen))
    {
        const char* names[ComponentParticleSystem::BlendCount] = { "Additive", "Alpha" };

        ImGui::Combo("Blend mode", (int*)&component->blend_mode, names, int(ComponentParticleSystem::BlendCount));
        ImGui::InputFloat("Layer", &component->layer, 0.1f);
        ImGui::Checkbox("Visible", &component->visible);
    }

}

UID PanelProperties::DrawResourceType(Resource::Type type, bool opened)
{
    static UID selected = 0;
	vector<const Resource*> resources;

	static const char* titles[] = {
		"Models", "Materials", "Textures", "Meshes", "Audios", "Animation", "State machines", "Others" };

    bool open_tree =ImGui::TreeNodeEx(titles[type], opened ? ImGuiTreeNodeFlags_DefaultOpen : 0);

    if (open_tree)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_LIGHT_GREY);
        bool remove = false;
        App->resources->GatherResourceType(resources, type);
        for (vector<const Resource*>::const_iterator it = resources.begin(); !remove && it != resources.end(); ++it)
        {
            const Resource* info = (*it);

            ImGui::PushID(info->GetExportedFile());

            if (ImGui::TreeNodeEx(info->GetUserResName(), info->GetUID() == selected ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_Leaf))
            {
                if (ImGui::IsItemClicked(0) )
                {
                    selected = info->GetUID();
                }

                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("UID: %llu", info->GetUID());
                    ImGui::Text("Source: %s", info->GetFile());
                    ImGui::Text("Exported: %s", info->GetExportedFile());
                    ImGui::Text("References: %u", info->CountReferences());
                    ImGui::EndTooltip();
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        ImGui::TreePop();
        ImGui::PopStyleColor();
    }

    return ImGui::IsMouseDoubleClicked(0) ? selected : 0;
}

