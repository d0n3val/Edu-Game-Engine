#include "Globals.h"

#include "DualKawaseBlur.h"

#include "OGL.h"
#include "OpenGL.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#include "Leaks.h"

DualKawaseBlur::DualKawaseBlur()
{
}

DualKawaseBlur::~DualKawaseBlur()
{
}

void DualKawaseBlur::execute(const Texture2D *input, uint internal_format, uint format, uint type, uint width, uint height)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "DualKawaseBlur");
    createPrograms();
    createFramebuffers(internal_format, format, type, width, height);

    // downscale
    downscaleProg->Use();

    // 1
    glViewport(0, 0, width / 2, height / 2);
    intermediateFB->Bind();
    input->Bind(DUALKAWASE_INPUT_BINDING);
    glDrawArrays(GL_TRIANGLES, 0, 3);


    // upscale
    upscaleProg->Use();

    // 1
    glViewport(0, 0, width, height);
    resultFB->Bind();
    intermediate->Bind(DUALKAWASE_INPUT_BINDING);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glPopDebugGroup();
}

void DualKawaseBlur::createFramebuffers(uint internal_format, uint format, uint type, uint width, uint height)
{
    if (internal_format != rInternal || format != rFormat || type != rType || width != rWidth || height != rHeight)
    {
        uint intermediateW = width /2;
        uint intermediateH = height /2 ;

        intermediate = std::make_unique<Texture2D>(intermediateW, intermediateH, internal_format, format, type, nullptr, false);

        if (!intermediateFB)
        {
            intermediateFB = std::make_unique<Framebuffer>();
        }

        intermediateFB->ClearAttachments();
        intermediateFB->AttachColor(intermediate.get(), 0, 0);
        assert(intermediateFB->Check() == GL_FRAMEBUFFER_COMPLETE);

        result = std::make_unique<Texture2D>(width, height, internal_format, format, type, nullptr, false);

        if (!resultFB)
        {
            resultFB = std::make_unique<Framebuffer>();
        }

        resultFB->ClearAttachments();
        resultFB->AttachColor(result.get(), 0, 0);
        assert(resultFB->Check() == GL_FRAMEBUFFER_COMPLETE);

        rInternal = internal_format;
        rFormat = format;
        rType = type;
        rWidth = width;
        rHeight = height;
    }
}

void DualKawaseBlur::createPrograms()
{
	if(!downscaleProg || !upscaleProg)
	{
        std::unique_ptr<Shader> vertex, downFrag, upFrag;

        vertex.reset(Shader::CreateVSFromFile("assets/shaders/fullscreenVS.glsl", 0, 0));

        bool ok = vertex->Compiled();

        if (ok)
        {
            downFrag.reset(Shader::CreateFSFromFile("assets/shaders/DualkawaseDownFS.glsl", 0, 0));
            upFrag.reset(Shader::CreateFSFromFile("assets/shaders/DualkawaseUpFS.glsl", 0, 0));

            ok = downFrag->Compiled() && upFrag->Compiled();
        }

        if (ok)
        {
            downscaleProg = std::make_unique<Program>(vertex.get(), downFrag.get(), "kawase downscale");
            upscaleProg = std::make_unique<Program>(vertex.get(), upFrag.get(), "kawase upscale");

            ok = downscaleProg->Linked() && upscaleProg->Linked();
        }

        if (!ok)
        {
            downscaleProg.release();
            upscaleProg.release();
        }
    }
}