#include "Globals.h"

#include "ComponentSpotCone.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ResourceMesh.h"
#include "GameObject.h"

#define DEFAULT_HEIGHT 1.0f
#define DEFAULT_RADIUS 0.2f
#define DEFAULT_SLICES 20
#define DEFAULT_STACKS 20


ComponentSpotCone::ComponentSpotCone(GameObject* go) : Component(go, Types::SpotCone)
{

}

ComponentSpotCone::~ComponentSpotCone()
{

}

void ComponentSpotCone::OnPlay() 
{

}

void ComponentSpotCone::OnStop() 
{

}

void ComponentSpotCone::OnUpdate(float dt) 
{

}

void ComponentSpotCone::OnSave(Config& config) const 
{
    config.AddUID("ConeMesh", cone);
    config.AddFloat("Height", height);
    config.AddFloat("Radius", radius);
}

void ComponentSpotCone::OnLoad(Config* config) 
{
    UID uid = config->GetUID("ConeMesh");
    height = config->GetFloat("Height", DEFAULT_HEIGHT);
    radius = config->GetFloat("Radius", DEFAULT_RADIUS);

    if(uid == 0 || App->resources->Get(uid) == nullptr)
    {
        uid = ResourceMesh::LoadCone(game_object->name.c_str(), height, radius, DEFAULT_SLICES, DEFAULT_STACKS);        
    }

    setCone(uid);
}

void ComponentSpotCone::setCone(UID uid)
{
    Resource* res = App->resources->Get(cone);

    if(res)
    {
        SDL_assert(res->GetType() == Resource::mesh);
        res->Release();
    }

    cone = uid;

    res = App->resources->Get(cone);

    if(res)
    {
        res->LoadToMemory();
    }
}

void ComponentSpotCone::reCreateMesh()
{
    ResourceMesh* res = static_cast<ResourceMesh*>(App->resources->Get(cone));

    if (res == nullptr)
    {
        setCone(ResourceMesh::LoadCone(game_object->name.c_str(), height, radius, DEFAULT_SLICES, DEFAULT_STACKS));
    }
    else
    {
        res->ReloadCone(height, radius, DEFAULT_SLICES, DEFAULT_STACKS);
    }
}

void ComponentSpotCone::GetBoundingBox(AABB &box) const 
{
    ResourceMesh* res = static_cast<ResourceMesh*>(App->resources->Get(cone));

    if(res != nullptr)
    {
        box.Enclose(res->bbox);
    }
}

const ResourceMesh* ComponentSpotCone::getMeshRes() const
{
	return static_cast<const ResourceMesh*>(App->resources->Get(cone));
}

void ComponentSpotCone::setHeight(float newHeight)
{
    if(newHeight != height)
    {
        height = newHeight;
        reCreateMesh();
    }
}

void ComponentSpotCone::setRadius(float newRadius)
{
    if(newRadius != radius)
    {
        radius = newRadius;
        reCreateMesh();
    }
}