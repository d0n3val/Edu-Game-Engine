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

    Plane planes[6];
    camera->frustum.GetPlanes(planes);

    float3 pos = camera->GetCameraMatrix().TranslatePart();

    CollectObjects(planes, pos, quadtree);
}

void RenderList::UpdateFrom(ComponentCamera* camera, GameObject* go)
{
    opaque_nodes.clear();
    transparent_nodes.clear();

    CollectObjects(camera, go);
}

void RenderList::CollectObjects(Plane* camera_planes, const float3& camera_pos, QuadtreeNode* quadtree)
{
    if (quadtree->box.IsFinite() && Intersects(camera_planes, quadtree->box))
    {
        for (GameObject* go : quadtree->objects)
        {
            AABB local_bounding = go->GetLocalBBox();

            bool inside = true;
            if (local_bounding.IsFinite())
            {
                float4x4 transform = go->GetGlobalTransformation();

                OBB global_bounding = local_bounding.Transform(transform);
                inside = Intersects(camera_planes, global_bounding);
            }

            if (inside)
            {
                CollectMeshRenderers(camera_pos, go);
                CollectParticleSystems(camera_pos, go);
                CollectTrails(camera_pos, go);
            }
        }

        for (QuadtreeNode* child : quadtree->childs)
        {
            if (child)
            {
                CollectObjects(camera_planes, camera_pos, child);
            }
        }
    }
}

bool RenderList::Intersects(Plane * camera_planes, const AABB & aabb)
{
    float3 points[8];
    aabb.GetCornerPoints(points);

    return Intersects(camera_planes, points);
}

bool RenderList::Intersects(Plane* camera_planes, const OBB& obb)
{
    float3 points[8];
    obb.GetCornerPoints(points);

    return Intersects(camera_planes, points);
}

bool RenderList::Intersects(Plane* camera_planes, const float3* points)
{
    int out;
    for (int i = 0; i < 6; ++i)
    {
        out = 0;
        for (int k = 0; k < 8; ++k)
            out += camera_planes[i].IsOnPositiveSide(points[k]);

        if (out == 8)
            return false;
    }

    return true;
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
                            //return info.distance < new_info.distance; 
                            return info.distance > new_info.distance || (info.distance == new_info.distance && info.layer <= new_info.layer);
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


