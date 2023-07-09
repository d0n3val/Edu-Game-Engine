#include "Globals.h"

#include "ComponentSpotCone.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
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
    timer.Start();
}

void ComponentSpotCone::OnStop() 
{
    timer.Stop();

}

void ComponentSpotCone::OnUpdate(float dt) 
{
}

void ComponentSpotCone::OnSave(Config& config) const 
{
    config.AddUID("ConeMesh", cone);
    config.AddUID("Fog0", fog0);
    config.AddFloat2("Fog0Tiling", fog0Tiling);
    config.AddFloat2("Fog1Tiling", fog1Tiling);
    config.AddFloat2("Fog0Offset", fog0Offset);
    config.AddFloat2("Fog1Offset", fog1Offset);
    config.AddFloat2("Fog0Speed", fog0Speed);
    config.AddFloat2("Fog1Speed", fog1Speed);
    config.AddUID("Fog1", fog1);
    config.AddFloat("Height", height);
    config.AddFloat("Radius", radius);
    config.AddFloat4("Colour", colour);
    config.AddFloat("Transparency", transparency);
    config.AddFloat("Smooth amount", smoothAmount);
    config.AddFloat("Fresnel amount", fresnelAmount);
}

void ComponentSpotCone::OnLoad(Config* config) 
{
    UID uid = config->GetUID("ConeMesh");

    setFog0(config->GetUID("Fog0"));
    setFog1(config->GetUID("Fog1"));
    fog0Tiling = config->GetFloat2("Fog0Tiling");
    fog1Tiling = config->GetFloat2("Fog1Tiling");
    fog0Offset = config->GetFloat2("Fog0Offset");
    fog1Offset = config->GetFloat2("Fog1Offset");
    fog0Speed  = config->GetFloat2("Fog0Speed");
    fog1Speed  = config->GetFloat2("Fog1Speed");
    colour     = config->GetFloat4("Colour", float4::one);

    height = config->GetFloat("Height", DEFAULT_HEIGHT);
    radius = config->GetFloat("Radius", DEFAULT_RADIUS);

    transparency = config->GetFloat("Transparency", 1.0);
    smoothAmount = config->GetFloat("Smooth amount", 1.0);
    fresnelAmount = config->GetFloat("Fresnel amount", 1.0);

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

void ComponentSpotCone::setFog0(UID texture)
{
    Resource* tex = App->resources->Get(fog0);
    if (tex)
    {
        tex->Release();
        fog0 = 0;
    }

    Resource* res = App->resources->Get(texture);

    if (res != nullptr && res->GetType() == Resource::texture && res->LoadToMemory() == true)
    {
        fog0 = texture;
    }
}

void ComponentSpotCone::setFog1(UID texture)
{
    Resource* tex = App->resources->Get(fog1);
    if (tex)
    {
        tex->Release();
        fog1 = 0;
    }

    Resource* res = App->resources->Get(texture);

    if (res != nullptr && res->GetType() == Resource::texture && res->LoadToMemory() == true)
    {
        fog1 = texture;
    }
}

const ResourceTexture* ComponentSpotCone::getFog0Res() const
{
    const Resource* res = App->resources->Get(fog0);
    if (res != nullptr && res->GetType() == Resource::texture)
    {
        return static_cast<const ResourceTexture*>(res);
    }

    return nullptr;
}

const ResourceTexture* ComponentSpotCone::getFog1Res() const
{
    Resource *res = App->resources->Get(fog1);
    if (res != nullptr && res->GetType() == Resource::texture)
    {
        return static_cast<const ResourceTexture*>(res);
    }

    return nullptr;
}

ResourceTexture* ComponentSpotCone::getFog0Res()
{
    Resource* res = App->resources->Get(fog0);
    if (res != nullptr && res->GetType() == Resource::texture)
    {
        return static_cast<ResourceTexture*>(res);
    }

    return nullptr;
}

ResourceTexture* ComponentSpotCone::getFog1Res()
{
    Resource* res = App->resources->Get(fog1);
    if (res != nullptr && res->GetType() == Resource::texture)
    {
        return static_cast<ResourceTexture*>(res);
    }

    return nullptr;
}
