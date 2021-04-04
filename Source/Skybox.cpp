#include "Globals.h"
#include "Skybox.h"

#include "Application.h"
#include "ModuleResources.h"
#include "ModulePrograms.h"
#include "Config.h"

#include "ResourceTexture.h"

#include "OpenGL.h"

Skybox::Skybox()
{
}

Skybox::~Skybox()
{
    if(cubemap)
    {
        Resource* res = App->resources->Get(cubemap);
        
        if (res)
        {
            res->Release();
        }
    }
}

void Skybox::Load(const Config& config)
{
    UID texture = config.GetUInt("Texture", 0);
    if (texture)
    {
        SetCubemap(texture);
    }
}

void Skybox::Save(Config& config) const
{
    config.AddUInt("Texture", uint(cubemap));
}

void Skybox::Draw(const float4x4& proj, const float4x4& view)
{
    if (test)
    {
        utils.RenderSkybox(test, proj, view);
    }
}

void Skybox::SetCubemap(UID uid)
{
    if(cubemap != 0)
    {
        Resource* res = App->resources->Get(cubemap);
        if(res) res->Release();
    }

    ResourceTexture* res = App->resources->GetTexture(uid);
    if(res && res->LoadToMemory())
    {      
        if(res->GetType() == ResourceTexture::TextureCube)
        {
            test = static_cast<TextureCube*>(res->GetTexture());
        }
        else
        {
            test = utils.ConvertToCubemap(static_cast<Texture2D*>(res->GetTexture()), 512, 512);
        }

        cubemap = uid;
    }
    else
    {
        LOG("UID %d is not a texture!!!", uid);
        cubemap = 0;
    }
}

