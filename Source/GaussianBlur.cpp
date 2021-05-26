#include "Globals.h"

#include "GaussianBlur.h"

#include "OGL.h"
#include "OpenGL.h"

#include "Leaks.h"

GaussianBlur::GaussianBlur()
{
    const char* horizontalMacros[] = { "#define HORIZONTAL 1 \n" };

    std::unique_ptr<Shader> fullScreenVS(Shader::CreateVSFromFile("assets/shaders/fullscreenVS.glsl"));
    std::unique_ptr<Shader> horizontalFS(Shader::CreateFSFromFile("assets/shaders/gaussian.glsl", horizontalMacros, 1));
    std::unique_ptr<Shader> verticalFS(Shader::CreateFSFromFile("assets/shaders/gaussian.glsl"));

    if(fullScreenVS->Compiled() && horizontalFS->Compiled())
    {
        horizontal = std::make_unique<Program>(fullScreenVS.get(), horizontalFS.get(), "Horizontal Gaussian program");
    }

    if(fullScreenVS->Compiled() && verticalFS->Compiled())
    {
        vertical = std::make_unique<Program>(fullScreenVS.get(), verticalFS.get(), "Vertical Gaussian program");
    }
}

void GaussianBlur::Execute(const Texture2D *input, const Texture2D* output, uint width, uint height)
{
    if (!frameBuffer)
    {
        frameBuffer = std::make_unique<Framebuffer>();
    }

    frameBuffer->ClearAttachments();
    frameBuffer->AttachColor(output, 0, 0);
    assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);

    frameBuffer->Bind();
    glViewport(0, 0, width, height);

    // horizontal pass
    horizontal->Use();
    horizontal->BindTextureFromName("image", 0, input);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // vertical pass
    vertical->Use();
    vertical->BindTextureFromName("image", 0, input);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
