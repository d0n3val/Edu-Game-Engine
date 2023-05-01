#include "Globals.h"
#include "IBLData.h"

#include "Application.h"
#include "ModuleResources.h"
#include "ModulePrograms.h"
#include "Config.h"

#include "ResourceTexture.h"

#include "OpenGL.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

uint IBLData::refCount = 0;
std::unique_ptr<Texture2D> IBLData::environmentBRDF;

IBLData::IBLData()
{
    ++refCount;
}

IBLData::~IBLData()
{
    if(--refCount == 0)
    {
        environmentBRDF.reset();
    }
}

void IBLData::Load(const Config& config)
{
    UID texture = config.GetUInt("Texture", 0);
    if (texture)
    {
        SetEnvironmentRes(texture);
    }
    else
    {
        SetEnvironmentRes(App->resources->GetDefaultSkybox()->GetUID());
    }
}

void IBLData::Save(Config& config) const
{
    config.AddUInt("Texture", uint(envRes.GetUID()));
}

bool IBLData::DrawEnvironment(const float4x4& proj, const float4x4& view)  
{
    if(envRes)
    {
        const ResourceTexture* res = envRes.GetPtr<ResourceTexture>();
        utils.RenderCubemap(static_cast<const TextureCube*>(res->GetTexture()), proj, view);

        return true;
    }
    else if (environment)
    {
        utils.RenderCubemap(environment, proj, view);

        return true;
    }

    return false;
}

void IBLData::DrawDiffuseIBL(const float4x4& proj, const float4x4& view)  
{
    if(diffuseIBL)
    {
        utils.RenderCubemap(diffuseIBL.get(), proj, view);
    }
}

void IBLData::DrawPrefilteredIBL(const float4x4& proj, const float4x4& view, float roughness) 
{
    if (prefilteredIBL)
    {
        utils.RenderCubemapLod(prefilteredIBL.get(), proj, view, float(prefilteredLevels-1) * roughness);
    }
}

void IBLData::SetEnvironmentRes(UID uid)
{
    envRes = 0;
    ResHandle handle(uid);
    ResourceTexture *textureRes = handle.GetPtr<ResourceTexture>();
    if(textureRes && textureRes->GetTexType() == ResourceTexture::TextureCube)
    {
        SetEnvironment(static_cast<TextureCube*>(textureRes->GetTexture()));
        envRes = handle;
    }
    else
    {
        envRes = 0;
    }
}

void IBLData::generateEnvironment(const float3& position, const Quat& rotation, float farPlane, uint resolution, uint numSamples, uint roughnessLevels)
{
    const int DEFAULT_IBL_SIZE = 512;

    SetEnvironment(utils.LocalIBL(position, rotation, farPlane, resolution));
}

void IBLData::SetEnvironment(TextureCube* env, uint resolution, uint numSamples, uint roughnessLevels)
{
    const int ENVIRONMENT_BRDF_RESOLUTION = 512;
    const int DIFFUSE_IBL_RESOLUTION = 512;
    const int PREFILTERED_IBL_RESOLUTION = 2048;
    const int PREFILTERED_IBL_LEVELS = 6; // 128x128

    envRes = 0;
    environment = env;
    prefilteredLevels = PREFILTERED_IBL_LEVELS; 

    prefilteredIBL.reset(utils.PrefilteredSpecular(environment, resolution, numSamples, roughnessLevels));
    diffuseIBL.reset(utils.DiffuseIBL(environment, resolution, numSamples));

    if(!environmentBRDF)
    {
        environmentBRDF.reset(utils.EnvironmentBRDF(ENVIRONMENT_BRDF_RESOLUTION, ENVIRONMENT_BRDF_RESOLUTION));
    }
}

void IBLData::Bind()
{
    if (diffuseIBL && prefilteredIBL && environmentBRDF)
    {
        glUniform1i(PREFILTERED_LOD_LEVELS_LOCATION, prefilteredLevels);
        diffuseIBL->Bind(DIFFUSE_IBL_TEX_BINDING);
        prefilteredIBL->Bind(PREFILTERED_IBL_TEX_BINDING);
        environmentBRDF->Bind(ENVIRONMENT_BRDF_TEX_BINDING);
    }
}