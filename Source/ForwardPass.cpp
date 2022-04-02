#include "Globals.h"

#include "ForwardPass.h"
#include "Application.h"
#include "BatchManager.h"
#include "ModuleRenderer.h"
#include "ModuleLevelManager.h"
#include "Skybox.h"

#include "OGL.h"
#include "OpenGL.h"

#include "Leaks.h"

ForwardPass::ForwardPass()
{
}

ForwardPass::~ForwardPass()
{
}

void ForwardPass::executeOpaque(const RenderList &objects, Framebuffer *target, uint width, uint height)
{
    UseProgram();

    App->level->GetSkyBox()->BindIBL();

    if (target)
    {
        target->Bind();
        glViewport(0, 0, width, height);
    }

    App->renderer->GetBatchManager()->Render(objects.GetOpaques(), true);

    if (target)
    {
        target->Unbind();
    }
}

void ForwardPass::executeTransparent(const RenderList &objects, Framebuffer *target, uint width, uint height)
{
    UseProgram();

    App->level->GetSkyBox()->BindIBL();

    if (target)
    {
        target->Bind();
        glViewport(0, 0, width, height);
    }

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    App->renderer->GetBatchManager()->Render(objects.GetTransparents(), true);

    glDisable(GL_BLEND);

    if (target)
    {
        target->Unbind();
    }
}

void ForwardPass::UseProgram()
{
	if(!program)
	{
		GenerateProgram();
	}

	if(program)
	{
		program->Use();
	}
}

bool ForwardPass::GenerateProgram()
{
	std::unique_ptr<Shader> vertex, fragment;

	vertex.reset(Shader::CreateVSFromFile("assets/shaders/defVS.glsl", 0, 0));

	bool ok = vertex->Compiled();

	if (ok)
	{
		fragment.reset(Shader::CreateFSFromFile("assets/shaders/defFS.glsl", 0, 0));

		ok = fragment->Compiled();
	}

	if (ok)
	{
		program = std::make_unique<Program>(vertex.get(), fragment.get(), "default");

		ok = program->Linked();
	}

	if (!ok)
	{
		program.release();
	}

	return ok;
}
