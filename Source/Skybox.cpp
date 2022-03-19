#include "Globals.h"
#include "Skybox.h"

#include "Application.h"
#include "ModuleResources.h"
#include "ModulePrograms.h"
#include "Config.h"

#include "ResourceTexture.h"

#include "OpenGL.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

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
    else
    {
        SetCubemap(App->resources->GetDefaultSkybox()->GetUID());
    }
}

void Skybox::Save(Config& config) const
{
    config.AddUInt("Texture", uint(cubemap));
}

bool Skybox::Render(const float4x4& proj, const float4x4& view)
{
    if(cubemap)
    {
        ResourceTexture* res = App->resources->GetTexture(cubemap);
        if (res && res->GetType() == ResourceTexture::TextureCube)
        {
            utils.RenderSkybox(static_cast<TextureCube*>(res->GetTexture()), proj, view);

            return true;
        }
    }

    return false;
}

void Skybox::DrawDiffuseIBL(const float4x4& proj, const float4x4& view)
{
    if(diffuseIBL)
    {
        utils.RenderSkybox(diffuseIBL.get(), proj, view);
    }
}

void Skybox::DrawPrefilteredIBL(const float4x4& proj, const float4x4& view, float roughness)
{
    if (prefilteredIBL)
    {
        utils.RenderSkyboxLod(prefilteredIBL.get(), proj, view, float(prefilteredLevels-1) * roughness);
    }
}

void Skybox::SetCubemap(UID uid)
{
    const int ENVIRONMENT_BRDF_RESOLUTION = 512;
    const int DIFFUSE_IBL_RESOLUTION = 512;
    const int PREFILTERED_IBL_RESOLUTION = 2048;
    const int PREFILTERED_IBL_LEVELS = 6; // 128x128

    if(cubemap != 0)
    {
        Resource* res = App->resources->Get(cubemap);
        if(res) res->Release();
    }

    ResourceTexture* res = App->resources->GetTexture(uid);
    if(res && res->LoadToMemory() && res->GetType() == ResourceTexture::TextureCube)
    {      
        cubemap = uid;
    }
    else
    {
        LOG("UID %d is not a texture!!!", uid);
        cubemap = App->resources->GetDefaultSkybox()->GetUID();
        res = App->resources->GetTexture(cubemap);        
    }

    if(res != nullptr)
    {
        assert(res->GetType() == ResourceTexture::TextureCube);

        prefilteredLevels = PREFILTERED_IBL_LEVELS; 

        diffuseIBL.reset(utils.DiffuseIBL(static_cast<TextureCube*>(res->GetTexture()), DIFFUSE_IBL_RESOLUTION, DIFFUSE_IBL_RESOLUTION));
        prefilteredIBL.reset(utils.PrefilteredSpecular(static_cast<TextureCube*>(res->GetTexture()), PREFILTERED_IBL_RESOLUTION, PREFILTERED_IBL_RESOLUTION, prefilteredLevels));

        if(!environmentBRDF)
        {
            environmentBRDF.reset(utils.EnvironmentBRDF(ENVIRONMENT_BRDF_RESOLUTION, ENVIRONMENT_BRDF_RESOLUTION));
        }
    }
}

void Skybox::BindIBL()
{
    if (diffuseIBL && prefilteredIBL && environmentBRDF)
    {
        glUniform1i(PREFILTERED_LOD_LEVELS_LOCATION, prefilteredLevels);

        glActiveTexture(GL_TEXTURE0 + DIFFUSE_IBL_TEX_BINDING);
        glBindTexture(GL_TEXTURE_CUBE_MAP, diffuseIBL->Id());
        glActiveTexture(GL_TEXTURE0 + PREFILTERED_IBL_TEX_BINDING);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilteredIBL->Id());
        glActiveTexture(GL_TEXTURE0 + ENVIRONMENT_BRDF_TEX_BINDING);
        glBindTexture(GL_TEXTURE_2D, environmentBRDF->Id());
    }
}