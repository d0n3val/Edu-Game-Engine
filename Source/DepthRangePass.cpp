#include "Globals.h"

#include "DepthRangePass.h"

#include "OGL.h"
#include "OpenGL.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#include <algorithm>

#define KERNEL_SIZE 32

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

    int reductionWidth = int((width+(KERNEL_SIZE-1))/KERNEL_SIZE);
    int reductionHeight = int((height+(KERNEL_SIZE-1))/KERNEL_SIZE);

    programRed->Use();
    depthBuffer->Bind(DEPTH_INTEXTURE_BINDING);
    texture0->BindImage(DEPTH_OUTIMAGE_BINDING, 0, false, 0, GL_WRITE_ONLY, GL_RG32F);
    programRed->BindUniformFromName("inWidth", int(width));
    programRed->BindUniformFromName("inHeight", int(height));

    uint numGroupsX = (reductionWidth + (DEPTHREDUCTION_GROUP_WIDTH - 1)) / DEPTHREDUCTION_GROUP_WIDTH;
    uint numGroupsY = (reductionHeight + (DEPTHREDUCTION_GROUP_HEIGHT - 1)) / DEPTHREDUCTION_GROUP_HEIGHT;
    glDispatchCompute(numGroupsX, numGroupsY, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    
    Texture* srcTexture = texture0.get();
    Texture* dstTexture = texture1.get();
    while(reductionWidth > 1 || reductionHeight > 1)
    {

        programRedGreen->Use();
        srcTexture->Bind(DEPTH_INTEXTURE_BINDING);
        dstTexture->BindImage(DEPTH_OUTIMAGE_BINDING, 0, false, 0, GL_WRITE_ONLY, GL_RG32F);
        programRedGreen->BindUniformFromName("inWidth", reductionWidth);
        programRedGreen->BindUniformFromName("inHeight", reductionHeight);

        reductionWidth  = int((reductionWidth + (KERNEL_SIZE-1)) / KERNEL_SIZE);
        reductionHeight = int((reductionHeight + (KERNEL_SIZE-1)) / KERNEL_SIZE);

        numGroupsX = (reductionWidth + (DEPTHREDUCTION_GROUP_WIDTH - 1)) / DEPTHREDUCTION_GROUP_WIDTH;
        numGroupsY = (reductionHeight + (DEPTHREDUCTION_GROUP_HEIGHT - 1)) / DEPTHREDUCTION_GROUP_HEIGHT;
        glDispatchCompute(numGroupsX, numGroupsY, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        std::swap(srcTexture, dstTexture);
    }
    
    glGetTextureSubImage(srcTexture->Id(), 0, 0, 0, 0, 1, 1, 1, GL_RG, GL_FLOAT, sizeof(float2), reinterpret_cast<void*>(&minMax));
    glPopDebugGroup();

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