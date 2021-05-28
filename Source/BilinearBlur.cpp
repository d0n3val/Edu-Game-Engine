#include "Globals.h"

#include "BilinearBlur.h"

#include "OGL.h"
#include "OpenGL.h"

#include "Leaks.h"

BilinearBlur::BilinearBlur()
{
    std::unique_ptr<Shader> fullScreenVS(Shader::CreateVSFromFile("assets/shaders/fullscreenVS.glsl"));
    std::unique_ptr<Shader> bilinearFS(Shader::CreateFSFromFile("assets/shaders/bilinear.glsl"));

    if(fullScreenVS->Compiled() && bilinearFS->Compiled())
    {
        program = std::make_unique<Program>(fullScreenVS.get(), bilinearFS.get(), "Horizontal Bilinear program");
    }
}

void BilinearBlur::Execute(const Texture2D *input, const Texture2D* output, uint width, uint height)
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

    program->Use();
    program->BindTextureFromName("image", 0, input);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
