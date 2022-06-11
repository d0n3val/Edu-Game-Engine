#include "Globals.h"

#include "FxaaPass.h"

#include "OGL.h"
#include "OpenGL.h"
#include "Application.h"
#include "modulehints.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#include "Leaks.h"

FxaaPass::FxaaPass()
{

}

FxaaPass::~FxaaPass()
{

}

void FxaaPass::execute(Texture2D* ldrOutput, unsigned width, unsigned height)
{
    generateFramebuffer(width, height);
    generateProgram();

    frameBuffer->Bind();
    
    glViewport(0, 0, width, height);    
    program->Use();
    ldrOutput->Bind(FXAA_LDR_BINDING);
    program->BindUniform(FXAA_SUBPIXELBLENDING_LOCATION, std::get<float>(App->hints->GetDHint(std::string("Fxaa subpixel blending"), 1.0f)));
    glDrawArrays(GL_TRIANGLES, 0, 3);

    frameBuffer->Unbind();
}

void FxaaPass::generateFramebuffer(uint width, uint height)
{
    if (width != fbWidth || height != fbHeight)
    {
        if (!frameBuffer)
        {
            frameBuffer = std::make_unique<Framebuffer>();
        }

        frameBuffer->ClearAttachments();

        output = std::make_unique<Texture2D>(width, height, GL_RGB8, GL_RGB, GL_UNSIGNED_INT, nullptr, false);

        frameBuffer->AttachColor(output.get(), 0, 0);

        assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);

        fbWidth = width;
        fbHeight = height;
    }

}

bool FxaaPass::generateProgram()
{
    if(!program)
    {
        std::unique_ptr<Shader> vertex, fragment;

        vertex.reset(Shader::CreateVSFromFile("assets/shaders/fullscreenVS.glsl", 0, 0));

        bool ok = vertex->Compiled();

        if (ok)
        {
            fragment.reset(Shader::CreateFSFromFile("assets/shaders/FxaaFS.glsl", 0, 0));

            ok = fragment->Compiled();
        }

        if (ok)
        {
            program = std::make_unique<Program>(vertex.get(), fragment.get(), "Fxaa");

            ok = program->Linked();
        }

        if (!ok)
        {
            program.release();
        }

        return ok;
    }

    return true;
}