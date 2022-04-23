#include "Globals.h"

#include "ComponentDecal.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ResourceTexture.h"

#include "GameObject.h"

ComponentDecal::ComponentDecal(GameObject* go) : Component(go, Decal)
{
}

ComponentDecal::~ComponentDecal()
{
    App->resources->ReleaseFromMemory(albedo);
    App->resources->ReleaseFromMemory(normal);
    App->resources->ReleaseFromMemory(specular);
}

void ComponentDecal::OnSave(Config &config) const 
{
    config.AddUID("Albedo", albedo);
    config.AddUID("Normal", normal);
    config.AddUID("Specular", specular);
    config.AddFloat("NormalStrength", normalStrength);
}

void ComponentDecal::OnLoad(Config *config) 
{
    albedo   = LoadTexToMemory(config->GetUID("Albedo", 0));
    normal   = LoadTexToMemory(config->GetUID("Normal", 0));
    specular = LoadTexToMemory(config->GetUID("Specular", 0));
    normalStrength = config->GetFloat("NormalStrength", 1.0f);
}

ResourceTexture* ComponentDecal::GetAlbedoRes()
{
	return static_cast<ResourceTexture*>(App->resources->Get(albedo));
}

ResourceTexture* ComponentDecal::GetNormalRes()
{
	return static_cast<ResourceTexture*>(App->resources->Get(normal));
}

ResourceTexture* ComponentDecal::GetSpecularRes()
{
	return static_cast<ResourceTexture*>(App->resources->Get(specular));
}

void ComponentDecal::SetAlbedo(UID uid)
{
    if(uid != albedo)
    {
        App->resources->ReleaseFromMemory(albedo);
        albedo = LoadTexToMemory(uid);
    }
}

void ComponentDecal::SetNormal(UID uid)
{
    if(uid != normal)
    {
        App->resources->ReleaseFromMemory(normal);
        normal = LoadTexToMemory(uid);
    }
}

void ComponentDecal::SetSpecular(UID uid)
{
    if(uid != specular)
    {
        App->resources->ReleaseFromMemory(specular);
        specular = LoadTexToMemory(uid);
    }
}

UID ComponentDecal::LoadTexToMemory(UID uid)
{
    Resource *res = App->resources->Get(uid);

    if (res != nullptr && res->GetType() == Resource::texture)
    {
        if (res->LoadToMemory() == true)
        {
            return uid;
        }
    }

    return 0;
}

void ComponentDecal::GetBoundingBox(AABB &box) const 
{
    float3 halfSize = game_object->GetLocalScale()*0.5f;

    AABB aabb(-halfSize, halfSize);

    OBB obb;
    obb.SetFrom(aabb, game_object->GetGlobalTransformation());
    box.SetFrom(obb);
}