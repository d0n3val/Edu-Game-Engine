#include "Globals.h"

#include "KawaseBlur.h"

#include "OGL.h"
#include "OpenGL.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#include "Leaks.h"

#include <SDL_assert.h>

KawaseBlur::KawaseBlur()
{
    vao = std::make_unique<VertexArray>();
}

KawaseBlur::~KawaseBlur()
{
}

void KawaseBlur::execute(const Texture2D *input, const Texture2D* output, uint inMip, uint inWidth, uint inHeight,
                         uint outMip, uint outWidth, uint outHeight, uint step)
{    
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "KawaseBlur");
    createProgram();

    if(!frameBuffer) frameBuffer = std::make_unique<Framebuffer>();

    frameBuffer->ClearAttachments();
    frameBuffer->AttachColor(output, 0, outMip);
    SDL_assert(frameBuffer->Check() == GL_FRAMEBUFFER_COMPLETE);

    frameBuffer->Bind();
    glViewport(0, 0, outWidth, outHeight);

    float2 invInputSize(1.0f / float(inWidth), 1.0f / float(inHeight));

    program->Use();
    program->BindUniform(KAWASE_INV_INPUT_SIZE_LOCATION, invInputSize);
    program->BindUniform(KAWASE_INPUT_LOD, int(inMip));
    program->BindUniform(KAWASE_STEP, int(step));

    input->Bind(KAWASE_INPUT_BINDING);
    vao->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glPopDebugGroup();
}

void KawaseBlur::createProgram()
{
	if(!program)
	{
        std::unique_ptr<Shader> vertex, fragment;

        vertex.reset(Shader::CreateVSFromFile("assets/shaders/fullscreenVS.glsl", 0, 0));

        bool ok = vertex->Compiled();

        if (ok)
        {
            fragment.reset(Shader::CreateFSFromFile("assets/shaders/kawaseFS.glsl", 0, 0));

            ok = fragment->Compiled();
        }

        if (ok)
        {
            program = std::make_unique<Program>(vertex.get(), fragment.get(), "kawase");

            ok = program->Linked();
        }

        if (!ok)
        {
            program.release();
        }
    }
}