#include "Globals.h"

#include "DualKawaseBlur.h"

#include "OGL.h"
#include "OpenGL.h"

#include "../Game/Assets/Shaders/LocationsAndBindings.h"

#include "Leaks.h"

DualKawaseBlur::DualKawaseBlur()
{
    vao = std::make_unique<VertexArray>();
}

DualKawaseBlur::~DualKawaseBlur()
{
}

void DualKawaseBlur::execute(const Texture2D *input, uint internal_format, uint format, uint type, uint width, uint height, uint steps)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "DualKawaseBlur");
    createPrograms();
    createFramebuffers(internal_format, format, type, width, height, steps);

    if (!intermediates.empty())
    {
        vao->Bind();

        // downscale
        downscaleProg->Use();
        input->Bind(DUALKAWASE_INPUT_BINDING);

        for (const auto& intermediate : intermediates)
        {
            intermediate.fb->Bind();
            glViewport(0, 0, intermediate.width, intermediate.height);
            glDrawArrays(GL_TRIANGLES, 0, 3);

            intermediate.texture->Bind(DUALKAWASE_INPUT_BINDING);
        }

        // upscale
        upscaleProg->Use();

        for (auto it = intermediates.rbegin(); (it + 1) != intermediates.rend(); ++it)
        {
            const Intermediate& current = *it;
            const Intermediate& next = *(it + 1);
            next.fb->Bind();
            glViewport(0, 0, next.width, next.height);

            current.texture->Bind(DUALKAWASE_INPUT_BINDING);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        resultFB->Bind();
        glViewport(0, 0, width, height);
        intermediates.front().texture->Bind(DUALKAWASE_INPUT_BINDING);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        vao->Unbind();
    }

    glPopDebugGroup();
}

void DualKawaseBlur::createFramebuffers(uint internal_format, uint format, uint type, uint width, uint height, uint steps)
{
    if (internal_format != rInternal || format != rFormat || type != rType || width != rWidth || height != rHeight || steps != intermediates.size())
    {
        intermediates.clear();
        intermediates.reserve(steps);

        uint intermediateW = width;
        uint intermediateH = height;
        for(uint i=0; i< steps; ++i)
        {
            intermediateW = intermediateW  >> 1;
            intermediateH = intermediateH  >> 1;

            Intermediate data;

            data.width = intermediateW;
            data.height = intermediateH;
            data.texture  = std::make_unique<Texture2D>(intermediateW, intermediateH, internal_format, format, type, nullptr, false);

            data.fb = std::make_unique<Framebuffer>();

            data.fb->ClearAttachments();
            data.fb->AttachColor(data.texture.get(), 0, 0);
            assert(data.fb->Check() == GL_FRAMEBUFFER_COMPLETE);

            intermediates.push_back(std::move(data));
        }

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