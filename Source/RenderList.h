#ifndef _RENDERLIST_H_
#define _RENDERLIST_H_

#include "Math.h"

class ComponentCamera;
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

    void UpdateFrom(ComponentCamera* camera, QuadtreeNode* quadtree);
    void UpdateFrom(ComponentCamera* camera, GameObject* go);

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

    void CollectObjects(Plane* camera_planes, const float3& camera_pos, QuadtreeNode* quadtree);
    void CollectObjects(ComponentCamera* camera, GameObject* go);

    bool Intersects(Plane* camera_planes, const AABB& aabb);
    bool Intersects(Plane* camera_planes, const OBB& obb);
    bool Intersects(Plane* camera_planes, const float3* points);
    void CollectMeshRenderers        (const float3& camera_pos, GameObject* go);
    void CollectParticleSystems      (const float3& camera_pos, GameObject* go);
    void CollectTrails               (const float3& camera_pos, GameObject* go);
    void CollectDecals               (const float3& camera_pos, GameObject* go);

public:

};

#endif /* _RENDERLIST_H_ */
