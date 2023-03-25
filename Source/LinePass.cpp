#include "Globals.h"

#include "LinePass.h"

#include "ComponentLine.h"
#include "ResourceTexture.h"

#include "OGL.h"
#include "OpenGL.h"
#include "RenderList.h"

LinePass::LinePass()
{

}

LinePass::~LinePass()
{

}

void LinePass::execute(const RenderList& objects, Framebuffer* frameBuffer, uint width, uint height)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "LinePass");
    if(!objects.GetLines().empty())
    {
        frameBuffer->Bind();
        glViewport(0, 0, width, height);
        UseProgram();

        // additive blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        for(const TRenderInfo& info : objects.GetLines())
        {
            program->BindUniformFromName("model", info.line->GetModelMatrix());
            program->BindUniformFromName("time", info.line->GetTime());
            program->BindUniformFromName("tiling", info.line->GetTiling());
            program->BindUniformFromName("offset", info.line->GetOffset());
            program->BindTextureFromName("colorTex", 0, info.line->GetTexture().GetPtr<ResourceTexture>()->GetTexture());

            info.line->GetVAO()->Bind();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        }

        glDisable(GL_BLEND);
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