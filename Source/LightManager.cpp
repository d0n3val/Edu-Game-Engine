#include "Globals.h"
#include "LightManager.h"
#include "DirLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "QuadLight.h"
#include "SphereLight.h"
#include "TubeLight.h"
#include "LocalIBLLight.h"

#include "Config.h"

#include "OpenGL.h"

#include "Leaks.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#include <SDL_assert.h>

LightManager::LightManager()
{
    directional = std::make_unique<DirLight>();
}

LightManager::~LightManager()
{
}

void LightManager::LoadLights(const Config &config)
{
    Config dirCfg = config.GetSection("Directional");
    directional->Load(dirCfg);

    uint count = config.GetArrayCount("Points");
    for(uint i=0; i< count; ++i)
    {
        std::unique_ptr<PointLight> point = std::make_unique<PointLight>();
        Config pointsCfg = config.GetArray("Points", i);
        point->Load(pointsCfg);

        points.push_back(std::move(point));
    }

    count = config.GetArrayCount("Spots");
    for(uint i=0; i< count; ++i)
    {
        std::unique_ptr<SpotLight> spot = std::make_unique<SpotLight>();
        Config spotsCfg = config.GetArray("Spots", i);
        spot->Load(spotsCfg);

        spots.push_back(std::move(spot));
    }

    count = config.GetArrayCount("Quads");
    for(uint i=0; i < count; ++i)
    {
        std::unique_ptr<QuadLight> quad = std::make_unique<QuadLight>();
        Config quadsCfg = config.GetArray("Quads", i);
        quad->Load(quadsCfg);

        quads.push_back(std::move(quad));
    }

    count = config.GetArrayCount("Spheres");
    for(uint i=0; i< count; ++i)
    {
        std::unique_ptr<SphereLight> sphere = std::make_unique<SphereLight>();
        Config spheresCfg = config.GetArray("Spheres", i);
        sphere->Load(spheresCfg);

        spheres.push_back(std::move(sphere));
    }

    count = config.GetArrayCount("Tubes");
    for(uint i=0; i< count; ++i)
    {
        std::unique_ptr<TubeLight> tube = std::make_unique<TubeLight>();
        Config tubesCfg = config.GetArray("Tubes", i);
        tube->Load(tubesCfg);

        tubes.push_back(std::move(tube));
    }

    count = config.GetArrayCount("LocalIBLs");
    for(uint i=0; i<count; ++i)
    {
        std::unique_ptr<LocalIBLLight> ibl = std::make_unique<LocalIBLLight>();
        Config iblsCfg = config.GetArray("LocalIBLs", i);
        ibl->Load(iblsCfg);

        ibls.push_back(std::move(ibl));
    }
}

void LightManager::SaveLights(Config& config) const
{
    Config dirCfg = config.AddSection("Directional");
    directional->Save(dirCfg);

    config.AddArray("Points");

    for(const std::unique_ptr<PointLight>& point : points)
    {
        Config pointConfig;
        point->Save(pointConfig);

        config.AddArrayEntry(pointConfig);
    }

    config.AddArray("Spots");

    for(const std::unique_ptr<SpotLight>& spot : spots)
    {
        Config spotConfig;
        spot->Save(spotConfig);

        config.AddArrayEntry(spotConfig);
    }

    config.AddArray("Quads");

    for(const std::unique_ptr<QuadLight>& quad: quads)
    {
        Config quadConfig;
        quad->Save(quadConfig);

        config.AddArrayEntry(quadConfig);
    }

    config.AddArray("Spheres");

    for(const std::unique_ptr<SphereLight>& sphere : spheres)
    {
        Config sphereConfig;
        sphere->Save(sphereConfig);

        config.AddArrayEntry(sphereConfig);
    }

    config.AddArray("Tubes");

    for(const std::unique_ptr<TubeLight>& tube : tubes)
    {
        Config tubeConfig;
        tube->Save(tubeConfig);

        config.AddArrayEntry(tubeConfig);
    }

    config.AddArray("LocalIBLs");

    for(const std::unique_ptr<LocalIBLLight>& ibl : ibls)
    {
        Config iblConfig;
        ibl->Save(iblConfig);

        config.AddArrayEntry(iblConfig);
    }
}

uint LightManager::AddPointLight()
{
    uint index = uint(points.size());
    points.push_back(std::make_unique<PointLight>());
    return index;
}

void LightManager::RemovePointLight(uint index)
{
    points.erase(points.begin()+index);
}

uint LightManager::AddSpotLight()
{
    uint index = uint(spots.size());
    spots.push_back(std::make_unique<SpotLight>());

    return index;
}

void LightManager::RemoveSpotLight(uint index)
{
    spots.erase(spots.begin()+index);
}

uint LightManager::AddQuadLight()
{
    uint index = uint(quads.size());
    quads.push_back(std::make_unique<QuadLight>());

    return index;
}

void LightManager::RemoveQuadLight(uint index)
{
    quads.erase(quads.begin()+index);
}

uint LightManager::AddSphereLight()
{
    uint index = uint(spheres.size());
    spheres.push_back(std::make_unique<SphereLight>());

    return index;

}

void LightManager::RemoveSphereLight(uint index)
{
    spheres.erase(spheres.begin()+index);
}

uint LightManager::AddTubeLight()
{
    uint index = uint(tubes.size());
    tubes.push_back(std::make_unique<TubeLight>());

    return index;
}

void LightManager::RemoveTubeLight(uint index)
{
    tubes.erase(tubes.begin()+index);
}

uint LightManager::AddLocalIBLLight()
{
    uint index = uint(ibls.size());
    ibls.push_back(std::make_unique<LocalIBLLight>());

    return index;
}

void LightManager::RemoveLocalIBLLight(uint index)
{
    ibls.erase(ibls.begin()+index);
}

void LightManager::generateIBLs()
{
    for(std::unique_ptr<LocalIBLLight>& light : ibls)
    {
        light->generate();
    }
}

void LightManager::UpdateGPUBuffers(bool disableIBL)
{
    static bool needsUpdate = true;

    if (!needsUpdate)
        return;
    frameCount = (frameCount + 1) % 2;

    // directional
    if(!directionalSSBO[frameCount])
    {
        directionalSSBO[frameCount] = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, sizeof(DirLightData), nullptr, true);
        directionalData[frameCount] = reinterpret_cast<DirLightData *>(directionalSSBO[frameCount]->MapRange(GL_MAP_WRITE_BIT, 0, sizeof(DirLightData)));
        glObjectLabel(GL_BUFFER, directionalSSBO[frameCount]->Id(), -1, "DirectionalLightSSBO");
    }

    DirLightData* directionalPtr = directionalData[frameCount]; 
    directionalPtr->dir = float4(directional->GetDir(), directional->GetAnisotropy());
    directionalPtr->color = float4(directional->GetColor(), directional->GetIntensity());

    if(uint(points.size()) > pointBufferSize[frameCount] || !pointLightSSBO[frameCount])
    {
        pointBufferSize[frameCount] = uint(points.size());
        pointLightSSBO[frameCount] = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, pointBufferSize[frameCount] * sizeof(PointLightData) + sizeof(int) * 4, nullptr, true);
        pointLightData[frameCount] = reinterpret_cast<PointLightSet*>(pointLightSSBO[frameCount]->MapRange(GL_MAP_WRITE_BIT, 0, pointBufferSize[frameCount] * sizeof(PointLightData) + sizeof(int) * 4));

        glObjectLabel(GL_BUFFER, pointLightSSBO[frameCount]->Id(), -1, "PointLightSSBO");
    }

    PointLightSet* pointPtr = pointLightData[frameCount]; 

    // point lights
    enabledPointSize = 0;

    for(const std::unique_ptr<PointLight>& light : points)
    {
        if(light->GetEnabled())
        {
            pointPtr->points[enabledPointSize].position    = float4(light->GetPosition(), light->GetRadius());
            pointPtr->points[enabledPointSize].color       = float4(light->GetColor(), light->GetIntensity());
            pointPtr->points[enabledPointSize].anisotropy  = light->GetAnisotropy();

            ++enabledPointSize;
        }
    }
    pointPtr->count = enabledPointSize;

    if(uint(spots.size()) > spotBufferSize[frameCount] || !spotLightSSBO[frameCount])
    {
        spotBufferSize[frameCount] = uint(spots.size());
        spotLightSSBO[frameCount] = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, spotBufferSize[frameCount] * sizeof(SpotLightData) + sizeof(int) * 4, nullptr, true);
        spotLightData[frameCount] = reinterpret_cast<SpotLightSet*>(spotLightSSBO[frameCount]->MapRange(GL_MAP_WRITE_BIT, 0, spotBufferSize[frameCount] * sizeof(SpotLightData) + sizeof(int) * 4));
    }

    SpotLightSet* spotPtr = spotLightData[frameCount]; 

    // spot lights
    enabledSpotSize = 0;

    for(const std::unique_ptr<SpotLight>& light : spots)
    {
        if(light->GetEnabled())
        {
            spotPtr->spots[enabledSpotSize].transform       = light->GetTransform().Transposed();
            spotPtr->spots[enabledSpotSize].color           = float4(light->GetColor()*light->GetIntensity(), light->GetAnisotropy());
            spotPtr->spots[enabledSpotSize].distance        = light->GetMaxDistance();
            spotPtr->spots[enabledSpotSize].inner           = cosf(light->GetInnerCutoff());
            spotPtr->spots[enabledSpotSize].outer           = cosf(light->GetOutterCutoff());
            spotPtr->spots[enabledSpotSize].radius          = tanf(light->GetOutterCutoff())*light->GetMaxDistance();
            spotPtr->spots[enabledSpotSize].hasShadow       = light->GetShadowTex() != nullptr ? 1 : 0;
            spotPtr->spots[enabledSpotSize].shadowViewProj  = light->GetShadowViewProj().Transposed();
            spotPtr->spots[enabledSpotSize].shadowMap       = light->GetShadowTex() ? light->GetShadowTex()->GetBindlessHandle() : 0;

            ++enabledSpotSize;
        }
    }

    spotPtr->count = enabledSpotSize;

    if(uint(quads.size()) > quadBufferSize[frameCount] || !quadLightSSBO[frameCount])
    {
        quadBufferSize[frameCount] = uint(quads.size());
        quadLightSSBO[frameCount] = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, quadBufferSize[frameCount] * sizeof(QuadLightData) + sizeof(int) * 4, nullptr, true);
        quadLightData[frameCount] = reinterpret_cast<QuadLightSet*>(quadLightSSBO[frameCount]->MapRange(GL_MAP_WRITE_BIT, 0, quadBufferSize[frameCount] * sizeof(QuadLightData) + sizeof(int) * 4));
    }

    QuadLightSet* quadPtr = quadLightData[frameCount]; 

    // quad lights
    enabledQuadSize = 0;

    for(const std::unique_ptr<QuadLight>& light : quads)
    {
        if(light->GetEnabled())
        {
            quadPtr->quads[enabledQuadSize].position    = float4(light->GetPosition(), 0.0);
            quadPtr->quads[enabledQuadSize].up          = float4(light->GetUp(), 0.0f);
            quadPtr->quads[enabledQuadSize].right       = float4(light->GetRight(), 0.0f);
            quadPtr->quads[enabledQuadSize].colour      = float4(light->GetColor(), 0.0f);
            quadPtr->quads[enabledQuadSize].size        = float4(light->GetSize(), 0.0f ,0.0f);

            ++enabledQuadSize;
        }
    }

    quadPtr->count = enabledQuadSize;

    if(uint(spheres.size()) > sphereBufferSize[frameCount] || !sphereLightSSBO[frameCount])
    {
        sphereBufferSize[frameCount] = uint(spheres.size());
        sphereLightSSBO[frameCount] = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, sphereBufferSize[frameCount] * sizeof(SphereLightData) + sizeof(int) * 4, nullptr, true);
        sphereLightData[frameCount] = reinterpret_cast<SphereLightSet*>(sphereLightSSBO[frameCount]->MapRange(GL_MAP_WRITE_BIT, 0, sphereBufferSize[frameCount] * sizeof(SphereLightData) + sizeof(int) * 4));
    }

    SphereLightSet* spherePtr = sphereLightData[frameCount]; 

    // sphere lights
    enabledSphereSize = 0;

    for(const std::unique_ptr<SphereLight>& light : spheres)
    {
        if(light->GetEnabled())
        {
            spherePtr->spheres[enabledSphereSize].position    = float4(light->GetPosition(), light->GetRadius());
            spherePtr->spheres[enabledSphereSize].colour      = float4(light->GetColor()*light->GetIntensity(), light->GetLightRadius());
            spherePtr->spheres[enabledSphereSize].anisotropy  = light->GetAnisotropy();

            ++enabledSphereSize;
        }
    }

    spherePtr->count = enabledSphereSize;


    if(uint(tubes.size()) > tubeBufferSize[frameCount] || !tubeLightSSBO[frameCount])
    {
        tubeBufferSize[frameCount] = uint(tubes.size());
        tubeLightSSBO[frameCount] = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, tubeBufferSize[frameCount] * sizeof(TubeLightData) + sizeof(int) * 4, nullptr, true);
        tubeLightData[frameCount] = reinterpret_cast<TubeLightSet*>(tubeLightSSBO[frameCount]->MapRange(GL_MAP_WRITE_BIT, 0, tubeBufferSize[frameCount] * sizeof(TubeLightData) + sizeof(int) * 4));
    }

    TubeLightSet* tubePtr = tubeLightData[frameCount]; 

    // tube lights
    enabledTubeSize = 0;

    for(const std::unique_ptr<TubeLight>& light : tubes)
    {
        if(light->GetEnabled())
        {
            tubePtr->tubes[enabledTubeSize].pos0 = float4(light->GetPosition0(), light->GetRadius());
            tubePtr->tubes[enabledTubeSize].pos1 = float4(light->GetPosition1(), light->GetAnisotropy());
            tubePtr->tubes[enabledTubeSize].colour = float4(light->GetColor()*light->GetIntensity(), light->GetAttRadius());

            ++enabledTubeSize;
        }
    }

    tubePtr->count = enabledTubeSize;

    if(uint(ibls.size()) > iblBufferSize[frameCount] || !iblLightSSBO[frameCount])
    {
        iblBufferSize[frameCount] = uint(ibls.size());
        iblLightSSBO[frameCount] = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT, iblBufferSize[frameCount] * sizeof(IBLLightData) + sizeof(int) * 4, nullptr, true);
        iblLightData[frameCount] = reinterpret_cast<IBLLightSet*>(iblLightSSBO[frameCount]->MapRange(GL_MAP_WRITE_BIT, 0, iblBufferSize[frameCount] * sizeof(IBLLightData) + sizeof(int) * 4));

        glObjectLabel(GL_BUFFER, iblLightSSBO[frameCount]->Id(), -1, "iblLightSSBO");

    }

    IBLLightSet* iblPtr = iblLightData[frameCount];


    // ibl lights
    enablediblSize = 0;

    if(!disableIBL)
    {
        for (const std::unique_ptr<LocalIBLLight> &light : ibls)
        {
            IBLData &iblData = light->getIBLData();

            if (light->GetEnabled() && iblData.GetDiffuseIBL() && iblData.GetPrefilterdIBL())
            {

                float4x4 toGlobal = light->GetTransform();
                float4x4 toLocal = toGlobal;
                toLocal.InverseOrthonormal();

                iblPtr->ibls[enablediblSize].position = float4(light->GetPosition(), iblData.GetIntensity());
                iblPtr->ibls[enablediblSize].toLocal = toLocal;

                iblPtr->ibls[enablediblSize].minParallax = float4(light->GetParallaxAABB().minPoint, 0.0f);
                iblPtr->ibls[enablediblSize].maxParallax = float4(light->GetParallaxAABB().maxPoint, 0.0f);
                iblPtr->ibls[enablediblSize].minInfluence = float4(light->GetInfluenceAABB().minPoint, 0.0f);
                iblPtr->ibls[enablediblSize].maxInfluence = float4(light->GetInfluenceAABB().maxPoint, 0.0f);
                iblPtr->ibls[enablediblSize].diffuse = iblData.GetDiffuseIBL()->GetBindlessHandle();
                iblPtr->ibls[enablediblSize].prefiltered = iblData.GetPrefilterdIBL()->GetBindlessHandle();

                ++enablediblSize;
            }
        }
    }

    iblPtr->count = enablediblSize;
}

void LightManager::Bind()
{
    directionalSSBO[frameCount]->BindToPoint(DIRLIGHT_SSBO_BINDING);
    pointLightSSBO[frameCount]->BindToPoint(POINTLIGHT_SSBO_BINDING);
    spotLightSSBO[frameCount]->BindToPoint(SPOTLIGHT_SSBO_BINDING);
    quadLightSSBO[frameCount]->BindToPoint(QUADLIGHT_SSBO_BINDING);
    sphereLightSSBO[frameCount]->BindToPoint(SPHERELIGHT_SSBO_BINDING);
    tubeLightSSBO[frameCount]->BindToPoint(TUBELIGHT_SSBO_BINDING);
    iblLightSSBO[frameCount]->BindToPoint(IBLLIGHT_SSBO_BINDING);
}