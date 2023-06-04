#include "Globals.h"

#include "DepthRangePass.h"

#include "OGL.h"
#include "OpenGL.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#include <algorithm>
#include <SDL_assert.h>

DepthRangePass::DepthRangePass()
{
}

DepthRangePass::~DepthRangePass()
{
}

void DepthRangePass::execute(Texture *depthBuffer, uint width, uint height)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "DepthRangePass");
    createTextures(width, height);
    generatePrograms();

    // Parallel reduction 

    programRed->Use();
    depthBuffer->Bind(DEPTH_INTEXTURE_BINDING);
    texture0->BindImage(DEPTH_OUTIMAGE_BINDING, 0, false, 0, GL_WRITE_ONLY, GL_RG32F);
    programRed->BindUniformFromName("inWidth", int(width));
    programRed->BindUniformFromName("inHeight", int(height));

    uint numGroupsX = (width + (DEPTHREDUCTION_GROUP_WIDTH - 1)) / DEPTHREDUCTION_GROUP_WIDTH;
    uint numGroupsY = (height + (DEPTHREDUCTION_GROUP_HEIGHT - 1)) / DEPTHREDUCTION_GROUP_HEIGHT;
    glDispatchCompute(numGroupsX, numGroupsY, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    
    Texture* srcTexture = texture0.get();
    Texture* dstTexture = texture1.get();
    while(numGroupsX > 1 || numGroupsY > 1)
    {

        programRedGreen->Use();
        srcTexture->Bind(DEPTH_INTEXTURE_BINDING);
        dstTexture->BindImage(DEPTH_OUTIMAGE_BINDING, 0, false, 0, GL_WRITE_ONLY, GL_RG32F);
        programRedGreen->BindUniformFromName("inWidth", int(numGroupsX));
        programRedGreen->BindUniformFromName("inHeight", int(numGroupsY));

        numGroupsX = (numGroupsX + (DEPTHREDUCTION_GROUP_WIDTH - 1)) / DEPTHREDUCTION_GROUP_WIDTH;
        numGroupsY = (numGroupsY + (DEPTHREDUCTION_GROUP_HEIGHT - 1)) / DEPTHREDUCTION_GROUP_HEIGHT;
        glDispatchCompute(numGroupsX, numGroupsY, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        std::swap(srcTexture, dstTexture);
    }
    
    glPopDebugGroup();
    result = srcTexture;
}


void DepthRangePass::generatePrograms()
{
    if(!programRed || !programRedGreen)
    {
        const char *redMacros[] = {"#define USE_RED_CHANNEL \n"};
        std::unique_ptr<Shader> shaderRed = std::make_unique<Shader>(GL_COMPUTE_SHADER, "assets/shaders/depthParallelReduction.glsl", redMacros, 1);
        std::unique_ptr<Shader> shaderRedGreen = std::make_unique<Shader>(GL_COMPUTE_SHADER, "assets/shaders/depthParallelReduction.glsl");

        bool ok = shaderRed->Compiled() && shaderRedGreen->Compiled();

        if (ok)
        {
            programRed = std::make_unique<Program>(shaderRed.get());

            ok = programRed->Linked();
        }

        if (ok)
        {
            programRedGreen = std::make_unique<Program>(shaderRedGreen.get());

            ok = programRedGreen->Linked();
        }

        if (!ok)
        {
            programRed.release();
            programRedGreen.release();
        }
    }
}

void DepthRangePass::updateMinMax() const
{
    SDL_assert(result);
    glGetTextureSubImage(result->Id(), 0, 0, 0, 0, 1, 1, 1, GL_RG, GL_FLOAT, sizeof(float2), reinterpret_cast<void*>(&minMax));
    result = nullptr;
}

void DepthRangePass::createTextures(uint width, uint height)
{
    width = (width+1) / 2;
    height = (height+1) / 2;
    if(width > texWidth || height > texHeight)
    {
        texture0 = std::make_unique<Texture2D>(width, height, GL_RG32F, GL_RG, GL_FLOAT, nullptr, false);
        texture1 = std::make_unique<Texture2D>(width, height, GL_RG32F, GL_RG, GL_FLOAT, nullptr, false);

        texWidth = width;
        texHeight = height;
    }
}