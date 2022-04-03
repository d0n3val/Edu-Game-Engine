#include "Globals.h"

#include "ComponentDecal.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ResourceTexture.h"

ComponentDecal::ComponentDecal(GameObject* go) : Component(go, Decal)
{
}

ComponentDecal::~ComponentDecal()
{
    App->resources->ReleaseFromMemory(albedo);
    App->resources->ReleaseFromMemory(normal);
    App->resources->ReleaseFromMemory(emissive);
}

void ComponentDecal::OnSave(Config &config) const 
{
    config.AddUID("Albedo", albedo);
    config.AddUID("Normal", normal);
    config.AddUID("Emissive", emissive);
}

void ComponentDecal::OnLoad(Config *config) 
{
    albedo   = LoadTexToMemory(config->GetUID("Albedo", 0));
    normal   = LoadTexToMemory(config->GetUID("Normal", 0));
    emissive = LoadTexToMemory(config->GetUID("Emissive", 0));
}

ResourceTexture* ComponentDecal::GetAlbedoRes()
{
	return static_cast<ResourceTexture*>(App->resources->Get(albedo));
}

ResourceTexture* ComponentDecal::GetNormalRes()
{
	return static_cast<ResourceTexture*>(App->resources->Get(normal));
}

ResourceTexture* ComponentDecal::GetEmissiveRes()
{
	return static_cast<ResourceTexture*>(App->resources->Get(emissive));
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

void ComponentDecal::SetEmissive(UID uid)
{
    if(uid != emissive)
    {
        App->resources->ReleaseFromMemory(emissive);
        emissive = LoadTexToMemory(uid);
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