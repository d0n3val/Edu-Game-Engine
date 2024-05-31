#include "Globals.h"
#include "TileCullingPass.h"

#include "Application.h"
#include "GBufferExportPass.h"
#include "ModuleLevelManager.h"
#include "ModuleRenderer.h"
#include "LightManager.h"
#include "GBufferExportPass.h"
#include "CameraUBO.h"

#include "OpenGL.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

TileCullingPass::TileCullingPass()
{

}

TileCullingPass::~TileCullingPass()
{

}

void TileCullingPass::execute()
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "TileCullingPass");

    GBufferExportPass* exportPass = App->renderer->GetGBufferExportPass();

    uint width = exportPass->getWidth();
    uint height = exportPass->getHeight();

    int numTilesX = (width + (TILE_CULLING_GROUP_SIZE - 1)) / TILE_CULLING_GROUP_SIZE;
    int numTilesY = (height + (TILE_CULLING_GROUP_SIZE - 1)) / TILE_CULLING_GROUP_SIZE;

    LightManager* lightManager = App->level->GetLightManager();

    int numPoint = int(lightManager->GetEnabledPointLights());
    int numSpot = int(lightManager->GetEnabledSpotLights());
    generateTextureBuffer(numTilesX, numTilesY, numPoint, numSpot);

    useProgram();

    exportPass->getDepth()->Bind(GBUFFER_DEPTH_TEX_BINDING);
    program->BindUniform(TILECULLING_WIDTH_LOCATION, int(width));
    program->BindUniform(TILECULLING_HEIGHT_LOCATION, int(height));
    program->BindUniform(TILECULLING_MAX_POINT_LIGHTS_LOCATION, numPoint);
    program->BindUniform(TILECULLING_MAX_SPOT_LIGHTS_LOCATION, numSpot);
    lightManager->Bind();
    App->renderer->GetCameraUBO()->Bind();
    pointListTex->BindImage(TILE_CULLING_POINTIMAGE_BINDING, 0, false, 0, GL_WRITE_ONLY, GL_R32I);
    spotListTex->BindImage(TILE_CULLING_SPOTIMAGE_BINDING, 0, false, 0, GL_WRITE_ONLY, GL_R32I);
    volSpotListTex->BindImage(TILE_CULLING_VOLSPOTIMAGE_BINDING, 0, false, 0, GL_WRITE_ONLY, GL_R32I);
    //dbgTex->BindImage(TILE_CULLING_DBGIMAGE_BINDING, 0, false, 0, GL_READ_WRITE, GL_RGBA32F);

    glDispatchCompute(numTilesX, numTilesY, 1);

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

    glPopDebugGroup();
}

void TileCullingPass::useProgram()
{
    if(!program)
    {
        std::unique_ptr<Shader> shader = std::make_unique<Shader>(GL_COMPUTE_SHADER, "assets/shaders/tileCulling.glsl");

        if(shader->Compiled())
        {
            program = std::make_unique<Program>(shader.get());
        }
    }

    if(program)
    {
        program->Use();
    }
}

void TileCullingPass::generateTextureBuffer(int tilesX, int tilesY, int numPoints, int numSpots)
{
    uint newPointSize = tilesX * tilesY * numPoints*sizeof(int32_t);
    uint newSpotSize = tilesX * tilesY * numSpots*sizeof(int32_t);


    if(newPointSize > pointSize)
    {
        pointListBuffer = std::make_unique<Buffer>(GL_TEXTURE_BUFFER, GL_STATIC_DRAW, newPointSize, nullptr);
        pointListTex = std::make_unique<TextureBuffer>(pointListBuffer.get(), GL_R32I);
        pointSize = newPointSize;
        
        glObjectLabel(GL_TEXTURE, pointListTex->Id(), -1, "PointLightList");

    }

    if(newSpotSize > spotSize)
    {
        spotListBuffer = std::make_unique<Buffer>(GL_TEXTURE_BUFFER, GL_STATIC_DRAW, newSpotSize, nullptr);
        volSpotListBuffer = std::make_unique<Buffer>(GL_TEXTURE_BUFFER, GL_STATIC_DRAW, newSpotSize, nullptr);

        spotListTex = std::make_unique<TextureBuffer>(spotListBuffer.get(), GL_R32I);
        volSpotListTex = std::make_unique<TextureBuffer>(volSpotListBuffer.get(), GL_R32I);

        spotSize = newSpotSize;

        glObjectLabel(GL_TEXTURE, spotListTex->Id(), -1, "SpotLightList");
        glObjectLabel(GL_TEXTURE, volSpotListTex->Id(), -1, "VolSpotLightList");

        //dbgBuffer = std::make_unique<Buffer>(GL_TEXTURE_BUFFER, GL_STATIC_DRAW, tilesX*tilesY*numSpots*sizeof(float)*4, nullptr);
        //dbgTex = std::make_unique<TextureBuffer>(dbgBuffer.get(), GL_RGBA32F);
    }
}