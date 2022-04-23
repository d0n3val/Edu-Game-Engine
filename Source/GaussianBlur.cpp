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

void GaussianBlur::execute(const Texture2D *input, const Texture2D* output, uint internal_format, uint format, uint type, uint width, uint height)
{
    if (!frameBufferH)
    {
        frameBufferH = std::make_unique<Framebuffer>();
    }

    if (!frameBufferV)
    {
        frameBufferV = std::make_unique<Framebuffer>();
    }

    createResult(internal_format, format, type, width, height);

    // first output should be result, second input should be result

    frameBufferH->ClearAttachments();
    frameBufferH->AttachColor(result.get(), 0, 0);
    assert(frameBufferH->Check() == GL_FRAMEBUFFER_COMPLETE);

    frameBufferV->ClearAttachments();
    frameBufferV->AttachColor(output, 0, 0);
    assert(frameBufferV->Check() == GL_FRAMEBUFFER_COMPLETE);

    frameBufferH->Bind();
    glViewport(0, 0, width, height);

    // horizontal pass
    horizontal->Use();
    horizontal->BindTextureFromName("image", 0, input);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    frameBufferV->Bind();
    glViewport(0, 0, width, height);
    // vertical pass
    vertical->Use();
    vertical->BindTextureFromName("image", 0, result.get());
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void GaussianBlur::createResult(uint internal_format, uint format, uint type, uint width, uint height)
{
    if (internal_format != rInternal || format != rFormat || type != rType || width != rWidth || height != rHeight)
    {
        result = std::make_unique<Texture2D>(width, height, internal_format, format, type, nullptr, false);
        rInternal = internal_format;
        rFormat = format;
        rType = type;
        rWidth = width;
        rHeight = height;
    }

}