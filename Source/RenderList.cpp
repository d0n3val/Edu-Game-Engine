#include "Globals.h"
#include "RenderList.h"

#include "Quadtree.h"
#include "ComponentCamera.h"
#include "ComponentMeshRenderer.h"
#include "ComponentParticleSystem.h"
#include "ComponentTrail.h"
#include "GameObject.h"

void RenderList::UpdateFrom(ComponentCamera* camera, QuadtreeNode* quadtree)
{
    opaque_nodes.clear();
    transparent_nodes.clear();

    CollectObjects(camera, quadtree);
}

void RenderList::UpdateFrom(ComponentCamera* camera, GameObject* go)
{
    opaque_nodes.clear();
    transparent_nodes.clear();

    CollectObjects(camera, go);
}

void RenderList::CollectObjects(ComponentCamera* camera, QuadtreeNode* quadtree)
{
    if(quadtree->box.IsFinite() && camera->frustum.Intersects(quadtree->box))
    {
        for(GameObject* go : quadtree->objects)
        {
            AABB local_bounding = go->GetLocalBBox();

            bool inside = true;
            if (local_bounding.IsFinite())
            {
                float4x4 transform = go->GetGlobalTransformation();

                OBB global_bounding = local_bounding.Transform(transform);
                inside = camera->frustum.Intersects(global_bounding);
            }

            if(inside)
            {
                float3 camera_pos = camera->GetCameraMatrix().TranslatePart();
                CollectMeshRenderers(camera_pos, go);
                CollectParticleSystems(camera_pos, go);
                CollectTrails(camera_pos, go);
            }
        }

        for(QuadtreeNode* child : quadtree->childs)
        {
            if(child)
            {
                CollectObjects(camera, child);
            }
        }
    }
}

void RenderList::CollectObjects(ComponentCamera* camera, GameObject* go)
{
    AABB local_bounding = go->GetLocalBBox();

    bool inside = true;
    if (local_bounding.IsFinite())
    {
        float4x4 transform = go->GetGlobalTransformation();

        OBB global_bounding = local_bounding.Transform(transform);
        inside = camera->frustum.Intersects(global_bounding);
    }

    if (inside)
    {
        float3 camera_pos = camera->GetCameraMatrix().TranslatePart();
        CollectMeshRenderers(camera_pos, go);
        CollectParticleSystems(camera_pos, go);
        CollectTrails(camera_pos, go);
    }

    for(auto it = go->childs.begin(), end = go->childs.end(); it != end; ++it)
    {
        CollectObjects(camera, *it);
    }
}

void RenderList::CollectMeshRenderers(const float3& camera_pos, GameObject* go)
{
    std::vector<Component*> components;
    go->FindComponents(Component::MeshRenderer, components);

    float distance = (go->global_bbox.CenterPoint()-camera_pos).LengthSq();

    for(Component* comp : components)
    {
        TRenderInfo render;
        render.name = go->name.c_str();
        render.go   = go;
        render.mesh = static_cast<ComponentMeshRenderer*>(comp);
        render.distance = distance;

        if(render.mesh->GetVisible())
        {
            if(render.mesh->RenderMode() == ComponentMeshRenderer::RENDER_OPAQUE)
            {
                NodeList::iterator it = std::lower_bound(opaque_nodes.begin(), opaque_nodes.end(), render, 
                        [](const TRenderInfo& info, const TRenderInfo& new_info) 
                        { 
                            return info.distance < new_info.distance; 
                        } );

                opaque_nodes.insert(it, render);
            }
            else
            {
                NodeList::iterator it = std::lower_bound(transparent_nodes.begin(), transparent_nodes.end(), render, 
                        [](const TRenderInfo& info, const TRenderInfo& new_info) 
                        { 
                            return info.distance > new_info.distance || (info.distance == new_info.distance && info.layer <= new_info.layer);
                        });

                transparent_nodes.insert(it, render);
            }
        }
    }
}

void RenderList::CollectParticleSystems(const float3& camera_pos, GameObject* go)
{
    std::vector<Component*> components;
    go->FindComponents(Component::ParticleSystem, components);

    float distance = (go->GetGlobalPosition()-camera_pos).LengthSq();

    for(Component* comp : components)
    {
        TRenderInfo render;
        render.name = go->name.c_str();
        render.go   = go;

        render.distance = distance;
        render.particles= static_cast<ComponentParticleSystem*>(comp);
        render.layer = render.particles->GetLayer();

        NodeList::iterator it = std::lower_bound(transparent_nodes.begin(), transparent_nodes.end(), render, 
                        [](const TRenderInfo& info, const TRenderInfo& new_info) 
                        { 
                            return info.distance > new_info.distance || (info.distance == new_info.distance && info.layer <= new_info.layer);
                        });
        transparent_nodes.insert(it, render);
    }
}

void RenderList::CollectTrails(const float3& camera_pos, GameObject* go)
{
    std::vector<Component*> components;
    go->FindComponents(Component::Trail, components);

    float distance = (go->GetGlobalPosition()-camera_pos).LengthSq();

    for(Component* comp : components)
    {
        TRenderInfo render;
        render.name = go->name.c_str();
        render.go   = go;

        render.distance = distance;
        render.trail = static_cast<ComponentTrail*>(comp);

        NodeList::iterator it = std::lower_bound(transparent_nodes.begin(), transparent_nodes.end(), render, 
                        [](const TRenderInfo& info, const TRenderInfo& new_info) 
                        { 
                            return info.distance > new_info.distance || (info.distance == new_info.distance && info.layer <= new_info.layer);
                        });
        transparent_nodes.insert(it, render);
    }
}


