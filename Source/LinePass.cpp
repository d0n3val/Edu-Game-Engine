#include "Globals.h"

#include "LinePass.h"

#include "ComponentLine.h"
#include "ResourceTexture.h"

#include "OGL.h"
#include "OpenGL.h"
#include "RenderList.h"

#define SHOW_BILLOARD 0

LinePass::LinePass()
{

}

LinePass::~LinePass()
{

}

void LinePass::execute(const ComponentCamera* camera, const RenderList& objects, Framebuffer* frameBuffer, uint width, uint height)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "LinePass");
    if(!objects.GetLines().empty())
    {
        frameBuffer->Bind();
        glViewport(0, 0, width, height);
        UseProgram();

        // additive blending
#if !SHOW_BILLOARD
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
#else
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif 
        glFrontFace(GL_CW);
        glDepthMask(GL_FALSE);

        for(const TRenderInfo& info : objects.GetLines())
        {
            if(info.line->GetState() != ComponentLine::STOPPED)
            {
                info.line->UpdateBuffers();
                program->BindUniformFromName("model", info.line->GetModelMatrix(camera));
                program->BindUniformFromName("time", info.line->GetTime());
                program->BindUniformFromName("state", int(info.line->GetState()));

                if(info.line->GetState() == ComponentLine::STARTING) program->BindUniformFromName("fadeTime", info.line->GetFadeInTime());
                else if(info.line->GetState() == ComponentLine::STOPPING) program->BindUniformFromName("fadeTime", info.line->GetFadeOutTime());

                program->BindUniformFromName("tiling", info.line->GetTiling());
                program->BindUniformFromName("offset", info.line->GetOffset());
                program->BindTextureFromName("colorTex", 0, info.line->GetTexture().GetPtr<ResourceTexture>()->GetTexture());

                info.line->GetVAO()->Bind();
                glDrawElements(GL_TRIANGLE_STRIP, info.line->GetNumIndices(), GL_UNSIGNED_INT, nullptr);
            }
        }

        glFrontFace(GL_CCW);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);

#if SHOW_BILLOARD
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif 
    }
    glPopDebugGroup();
}

void LinePass::UseProgram()
{
    if(!program)
    {
        GenerateProgram();
    }

    program->Use();
}

void LinePass::GenerateProgram()
{
	std::unique_ptr<Shader> vertex, fragment;

	vertex.reset(Shader::CreateVSFromFile("assets/shaders/lineVS.glsl", 0, 0));

	bool ok = vertex->Compiled();

	if (ok)
	{

        fragment.reset(Shader::CreateFSFromFile("assets/shaders/lineFS.glsl", 0, 0));

		ok = fragment->Compiled();
	}

	if (ok)
	{
		program = std::make_unique<Program>(vertex.get(), fragment.get(), "Line");

		ok = program->Linked();
	}

	if (!ok)
	{
		program.release();
	}
}