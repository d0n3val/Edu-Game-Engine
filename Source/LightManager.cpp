#include "Globals.h"
#include "LightManager.h"
#include "DirLight.h"
#include "PointLight.h"
#include "SpotLight.h"

#include "Config.h"

#include "OpenGL.h"

#include "Leaks.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

LightManager::LightManager()
{
    directional = std::make_unique<DirLight>();
}

LightManager::~LightManager()
{
}

void LightManager::LoadLights(const Config &config)
{
    directional->Load(config.GetSection("Directional"));

    uint count = config.GetArrayCount("Points");
    for(uint i=0; i< count; ++i)
    {
        std::unique_ptr<PointLight> point = std::make_unique<PointLight>();
        point->Load(config.GetArray("Points", i));

        points.push_back(std::move(point));
    }

    count = config.GetArrayCount("Spots");
    for(uint i=0; i< count; ++i)
    {
        std::unique_ptr<SpotLight> spot = std::make_unique<SpotLight>();
        spot->Load(config.GetArray("Spots", i));

        spots.push_back(std::move(spot));
    }
}

void LightManager::SaveLights(Config& config) const
{
    directional->Save(config.AddSection("Directional"));

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

void LightManager::UpdateGPUBuffers()
{
    // directional
    if(!directionalSSBO)
    {
        directionalSSBO = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT, sizeof(DirLightData), nullptr, true);
    }

    DirLightData* directionalPtr = reinterpret_cast<DirLightData *>(directionalSSBO->MapRange(GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT, 0, sizeof(DirLightData)));
    directionalPtr->dir = float4(directional->GetDir(), 0.0);
    directionalPtr->color = float4(directional->GetColor(), directional->GetIntensity());
    directionalSSBO->Unmap();

    if(uint(points.size()) > pointBufferSize || !pointLightSSBO)
    {
        pointBufferSize = uint(points.size());
        pointLightSSBO = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT, pointBufferSize*sizeof(PointLightData)+sizeof(int)*4, nullptr, true);
    }

    PointLightSet* pointPtr = reinterpret_cast<PointLightSet *>(pointLightSSBO->MapRange(GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT, 0, pointBufferSize*sizeof(PointLightData)+sizeof(int)*4));

    // point lights
    uint index = 0;

    for(const std::unique_ptr<PointLight>& light : points)
    {
        if(light->GetEnabled())
        {
            pointPtr->points[index].position = light->GetPosition();
            pointPtr->points[index].radius   = light->GetRadius();
            pointPtr->points[index].color    = float4(light->GetColor(), light->GetIntensity());

            ++index;
        }
    }
    pointPtr->count = index;

    pointLightSSBO->Unmap();

    if(uint(spots.size()) > spotBufferSize || !spotLightSSBO)
    {
        spotBufferSize = uint(spots.size());
        spotLightSSBO = std::make_unique<Buffer>(GL_SHADER_STORAGE_BUFFER, GL_MAP_WRITE_BIT, spotBufferSize*sizeof(SpotLightData)+sizeof(int)*4, nullptr, true);
    }

    SpotLightSet* spotPtr = reinterpret_cast<SpotLightSet*>(spotLightSSBO->MapRange(GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT, 0, spotBufferSize*sizeof(SpotLightData)+sizeof(int)*4));

    // spot lights
    index = 0;

    for(const std::unique_ptr<SpotLight>& light : spots)
    {
        if(light->GetEnabled())
        {
            spotPtr->spots[index].position    = float4(light->GetPosition(), 0.0);
            spotPtr->spots[index].direction   = float4(light->GetDirection(), 0.0f);
            spotPtr->spots[index].color       = float4(light->GetColor(), 0.0f);
            spotPtr->spots[index].distance    = light->GetDistance();
            spotPtr->spots[index].inner       = cosf(light->GetInnerCutoff());
            spotPtr->spots[index].outer       = cosf(light->GetOutterCutoff());
            spotPtr->spots[index].intensity   = light->GetIntensity();

            ++index;
        }
    }

    spotPtr->count = index;

    spotLightSSBO->Unmap();
}

void LightManager::Bind()
{
    directionalSSBO->BindToPoint(DIRLIGHT_SSBO_BINDING);
    pointLightSSBO->BindToPoint(POINTLIGHT_SSBO_BINDING);
    spotLightSSBO->BindToPoint(SPOTLIGHT_SSBO_BINDING);
}