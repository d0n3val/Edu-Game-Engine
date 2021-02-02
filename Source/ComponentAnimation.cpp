#include "Globals.h"
#include "ComponentAnimation.h"
#include "ComponentMeshRenderer.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ResourceAnimation.h"
#include "ResourceMesh.h"
#include "ResourceStateMachine.h"
#include "AnimController.h"
#include "gameObject.h"
#include "Component.h"

#include <list>

#include "Leaks.h"

using namespace std;

// ---------------------------------------------------------
ComponentAnimation::ComponentAnimation(GameObject* container) : Component(container, Types::Animation)
{
    controller = new AnimController;
}

// ---------------------------------------------------------
ComponentAnimation::~ComponentAnimation()
{
    delete controller;
	
    if(context)
    {
        ax::NodeEditor::DestroyEditor(context);
        context = nullptr;
    }
}

// ---------------------------------------------------------
void ComponentAnimation::OnPlay()
{
    PlayNode(GetResource()->GetDefaultNode(), 0);
}

// ---------------------------------------------------------
void ComponentAnimation::ResetState()
{
    controller->Stop();
    PlayNode(GetResource()->GetDefaultNode(), 0);
}

// ---------------------------------------------------------
void ComponentAnimation::OnStop()
{
    controller->Stop();
    active_node = 0;
}

// ---------------------------------------------------------
void ComponentAnimation::OnUpdate(float dt)
{
    controller->Update(unsigned(dt*1000));

    if(game_object != nullptr)
    {
        UpdateGO(game_object);
    }
}

// ---------------------------------------------------------
void ComponentAnimation::OnSave(Config& config) const 
{
	config.AddUID("Resource", resource);
    config.AddBool("DebugDraw", debug_draw);
}

// ---------------------------------------------------------
void ComponentAnimation::OnLoad(Config* config) 
{
    SetResource(config->GetUID("Resource", 0));
    debug_draw = config->GetBool("DebugDraw", false);
}

// ---------------------------------------------------------
void ComponentAnimation::UpdateGO(GameObject* go)
{
    // Rígid update

    float3 position;
    Quat rotation;

    if(controller->GetTransform(go->name.c_str(), position, rotation))
    {
        go->SetLocalPosition(position);
        go->SetLocalRotation(rotation);
    }

    // Morph targets update
    
    tmp_components.clear();
    go->FindComponents(Component::MeshRenderer, tmp_components);

    for(Component* component : tmp_components)
    {
        ComponentMeshRenderer* mesh_renderer = static_cast<ComponentMeshRenderer*>(component);
        const ResourceMesh* mesh             = mesh_renderer->GetMeshRes();
        HashString morph_name                = HashString(mesh->GetName());
        uint num_morphs                      = mesh->GetNumMorphTargets();

        if(num_morphs > 0)
        {
            tmp_weights.resize(num_morphs, 0.0f);
            if (controller->GetWeights(morph_name, &tmp_weights[0], num_morphs))
            {
                for (uint i=0; i< num_morphs; ++i)
                {
                    mesh_renderer->SetMorphTargetWeight(i, tmp_weights[i]);
                }
            }
        }
    }

    for(std::list<GameObject*>::iterator it = go->childs.begin(), end = go->childs.end(); it != end; ++it)
    {
        UpdateGO(*it);
    }
}

// ---------------------------------------------------------
bool ComponentAnimation::SetResource(UID uid)
{
    Resource* res = App->resources->Get(uid);

    if (res != nullptr && res->GetType() == Resource::state_machine)
    {
        if(res->LoadToMemory() == true)
        {
            resource = uid;

            return true;
        }
    }

    return false;
}

// ---------------------------------------------------------
const ResourceStateMachine* ComponentAnimation::GetResource () const
{
    return static_cast<const ResourceStateMachine*>(App->resources->Get(resource));
}

// ---------------------------------------------------------
ResourceStateMachine* ComponentAnimation::GetResource ()
{
    return static_cast<ResourceStateMachine*>(App->resources->Get(resource));
}

// ---------------------------------------------------------
HashString ComponentAnimation::GetActiveNode() const
{
    const ResourceStateMachine* res = GetResource();

    if(res != nullptr && active_node <  res->GetNumNodes() )
    {
        return res->GetNodeName(active_node);
    }

    return HashString();
}

// ---------------------------------------------------------
void ComponentAnimation::SendTrigger(const HashString& trigger) 
{
    const ResourceStateMachine* state_res = GetResource();
    HashString active = GetActiveNode();

    for(uint i=0; i< state_res->GetNumTransitions(); ++i)
    {
        if(state_res->GetTransitionSource(i) == active && state_res->GetTransitionTrigger(i) == trigger)
        {
            PlayNode(state_res->GetTransitionTarget(i), state_res->GetTransitionBlend(i));
        }
    }
}

// ---------------------------------------------------------
void ComponentAnimation::PlayNode(const HashString& node, uint blend)
{
    PlayNode(GetResource()->FindNode(node), blend);
}

// ---------------------------------------------------------
void ComponentAnimation::PlayNode(uint node_idx, uint blend)
{
    ResourceStateMachine* state_res = GetResource();

    if(node_idx < state_res->GetNumNodes())
    {
        active_node   = node_idx;
        uint clip_idx = state_res->FindClip(state_res->GetNodeClip(node_idx));

        if(clip_idx < state_res->GetNumClips())
        {
            UID anim_res = state_res->GetClipRes(clip_idx);

            if(anim_res != 0)
            {
                controller->Play(anim_res, state_res->GetClipLoop(clip_idx), blend);
            }
        }
    }
}

// ---------------------------------------------------------
ComponentAnimation::EditorContext* ComponentAnimation::GetEditorContext()
{
    if(context == nullptr)
    {
        char* tmp = (char*)malloc(sizeof(char)*255);

		Resource* res = GetResource();
        sprintf_s(tmp, 255, ".%s%s.json", App->resources->GetDirByType(res->GetType()), res->GetExportedFile());

        ax::NodeEditor::Config cfg;
        cfg.SettingsFile = tmp;
        context = ax::NodeEditor::CreateEditor(&cfg);
    }

    return context;
}

