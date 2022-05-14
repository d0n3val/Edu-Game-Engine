#ifndef _RENDERLIST_H_
#define _RENDERLIST_H_

#include "Math.h"

class QuadtreeNode;
class GameObject;
class ComponentMeshRenderer;
class ComponentParticleSystem;
class ComponentTrail;
class ComponentDecal;

struct TRenderInfo
{
    const char*  name         = nullptr;
    GameObject*  go           = nullptr;
    union
    {
        ComponentMeshRenderer *mesh = nullptr;
        ComponentParticleSystem *particles;
        ComponentDecal* decal;
        ComponentTrail *trail;
    };
    float distance     = 0.0f;
    float layer        = 0.0f;
};

enum RenderListObjTypes
{
    RENDERLIST_OBJ_OPAQUE = 1 << 0,
    RENDERLIST_OBJ_TRANSPARENT = 1 << 1,
    RENDERLIST_OBJ_PARTICLES = 1 << 2,
    RENDERLIST_OBJ_TRAILS = 1 << 3,
    RENDERLIST_OBJ_DECALS = 1 << 4,
    RENDERLIST_OBJ_MESH = (RENDERLIST_OBJ_OPAQUE | RENDERLIST_OBJ_TRANSPARENT),
    RENDERLIST_OBJ_ALL = (RENDERLIST_OBJ_OPAQUE | RENDERLIST_OBJ_TRANSPARENT | RENDERLIST_OBJ_PARTICLES | 
                          RENDERLIST_OBJ_TRAILS | RENDERLIST_OBJ_DECALS)
};

typedef std::vector<TRenderInfo> NodeList;

class RenderList
{
private:

    NodeList opaque_nodes;
    NodeList transparent_nodes;
    NodeList particles;
    NodeList trails;
    NodeList decals;

public:

    void UpdateFrom(const Frustum& frustum, QuadtreeNode* quadtree, uint objTypes = RENDERLIST_OBJ_ALL);
    void UpdateFrom(const Frustum& frustum, GameObject* go, uint objTypes = RENDERLIST_OBJ_ALL);

    NodeList&       GetOpaques() { return opaque_nodes; }
    const NodeList& GetOpaques() const { return opaque_nodes; }

    NodeList&       GetTransparents() { return transparent_nodes; }
    const NodeList& GetTransparents() const { return transparent_nodes; }

    NodeList&       GetParticles() { return particles; }
    const NodeList& GetParticles() const { return particles; }

    NodeList&       GetTrails() { return trails; }
    const NodeList& GetTrails() const { return trails; }

    NodeList&       GetDecals() { return decals; }
    const NodeList& GetDecals() const { return decals; }

private:

    void CollectObjects(Plane* camera_planes, const float3& camera_pos, QuadtreeNode* quadtree, uint objTypes);
    void CollectObjects(const Frustum& frustum, GameObject* go, uint objTypes);

    bool Intersects(Plane* camera_planes, const AABB& aabb);
    bool Intersects(Plane* camera_planes, const OBB& obb);
    bool Intersects(Plane* camera_planes, const float3* points);
    void CollectMeshRenderers        (const float3& camera_pos, GameObject* go, uint objTypes);
    void CollectParticleSystems      (const float3& camera_pos, GameObject* go);
    void CollectTrails               (const float3& camera_pos, GameObject* go);
    void CollectDecals               (const float3& camera_pos, GameObject* go);

public:

};

#endif /* _RENDERLIST_H_ */
