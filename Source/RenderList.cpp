#include "Globals.h"
#include "RenderList.h"

#include "Quadtree.h"
#include "ComponentMeshRenderer.h"
#include "ComponentParticleSystem.h"
#include "ComponentTrail.h"
#include "ComponentDecal.h"
#include "GameObject.h"

void RenderList::UpdateFrom(const Frustum& frustum, QuadtreeNode* quadtree, uint objTypes /*= RENDERLIST_OBJ_ALL*/)
{
    opaque_nodes.clear();
    transparent_nodes.clear();
    particles.clear();
    trails.clear();
    decals.clear();

    Plane planes[6];
    frustum.GetPlanes(planes);

    float3 pos = frustum.WorldMatrix().TranslatePart();

    CollectObjects(planes, pos, quadtree, objTypes);
}

void RenderList::UpdateFrom(const Frustum& frustum, GameObject* go, uint objTypes /*= RENDERLIST_OBJ_ALL*/)
{
    opaque_nodes.clear();
    transparent_nodes.clear();
    particles.clear();
    trails.clear();
    decals.clear();

    Plane cameraPlanes[8];
    frustum.GetPlanes(cameraPlanes);

    CollectObjects(cameraPlanes, frustum.pos, go, objTypes);
}

void RenderList::UpdateFrom(const Plane *cameraPlanes, const float3 &cameraPos, GameObject *go, uint objTypes /*= RENDERLIST_OBJ_ALL*/)
{
    opaque_nodes.clear();
    transparent_nodes.clear();
    particles.clear();
    trails.clear();
    decals.clear();

    CollectObjects(cameraPlanes, cameraPos, go, objTypes);
}

void RenderList::CollectObjects(const Plane* camera_planes, const float3& camera_pos, QuadtreeNode* quadtree, uint objTypes)
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
                if((objTypes & RENDERLIST_OBJ_MESH) != 0)
                {
                    CollectMeshRenderers(camera_pos, go, objTypes);
                }

                if((objTypes & RENDERLIST_OBJ_PARTICLES) != 0)
                {
                    CollectParticleSystems(camera_pos, go);
                }

                if((objTypes & RENDERLIST_OBJ_TRAILS) != 0)
                {
                    CollectTrails(camera_pos, go);
                }

                if((objTypes & RENDERLIST_OBJ_DECALS) != 0)
                {
                    CollectDecals(camera_pos, go);
                }
            }
        }

        for (QuadtreeNode* child : quadtree->childs)
        {
            if (child)
            {
                CollectObjects(camera_planes, camera_pos, child, objTypes);
            }
        }
    }
}

bool RenderList::Intersects(const Plane * camera_planes, const AABB & aabb)
{
    float3 points[8];
    aabb.GetCornerPoints(points);

    return Intersects(camera_planes, points);
}

bool RenderList::Intersects(const Plane* camera_planes, const OBB& obb)
{
    float3 points[8];
    obb.GetCornerPoints(points);

    return Intersects(camera_planes, points);
}

bool RenderList::Intersects(const Plane* camera_planes, const float3* points)
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

void RenderList::CollectObjects(const Plane* camera_planes, const float3& camera_pos, GameObject* go, uint objTypes)
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
        if ((objTypes & RENDERLIST_OBJ_MESH) != 0)
        {
            CollectMeshRenderers(camera_pos, go, objTypes);
        }
        if ((objTypes & RENDERLIST_OBJ_PARTICLES) != 0)
        {
            CollectParticleSystems(camera_pos, go);
        }
        if((objTypes & RENDERLIST_OBJ_TRAILS) != 0)
        {
            CollectTrails(camera_pos, go);
        }
        if ((objTypes & RENDERLIST_OBJ_DECALS) != 0)
        {
            CollectDecals(camera_pos, go);
        }
    }

    for(auto it = go->childs.begin(), end = go->childs.end(); it != end; ++it)
    {
        CollectObjects(camera_planes, camera_pos, *it, objTypes);
    }
}

void RenderList::CollectMeshRenderers(const float3& camera_pos, GameObject* go, uint objType)
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
                if((objType & RENDERLIST_OBJ_OPAQUE) != 0)
                {
                    NodeList::iterator it = std::lower_bound(opaque_nodes.begin(), opaque_nodes.end(), render,
                                                             [](const TRenderInfo &info, const TRenderInfo &new_info)
                                                             {
                                                                 // return info.distance < new_info.distance;
                                                                 return info.distance > new_info.distance || (info.distance == new_info.distance && info.layer <= new_info.layer);
                                                             });

                    opaque_nodes.insert(it, render);
                }
            }
            else
            {
                if((objType & RENDERLIST_OBJ_TRANSPARENT) != 0)
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
        particles.insert(it, render);
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
        trails.insert(it, render);
    }
}

void RenderList::CollectDecals(const float3& camera_pos, GameObject* go)
{
    std::vector<Component*> components;
    go->FindComponents(Component::Decal, components);

    float distance = (go->GetGlobalPosition()-camera_pos).LengthSq();

    for(Component* comp : components)
    {
        TRenderInfo render;
        render.name = go->name.c_str();
        render.go   = go;

        render.distance = distance;
        render.decal = static_cast<ComponentDecal*>(comp);

        decals.push_back(render);
    }
}



