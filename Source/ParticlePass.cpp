#include "Globals.h"

#include "ParticlePass.h"

#include "ComponentParticleSystem.h"
#include "RenderList.h"
#include "OGL.h"
#include "OpenGL.h"

ParticlePass::ParticlePass()
{

}

ParticlePass::~ParticlePass()
{

}

void ParticlePass::execute(const ComponentCamera *camera, const RenderList &objects, Framebuffer *frameBuffer, uint width, uint height)
{
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "ParticlePass");
    if(!objects.GetParticles().empty())
    {
        frameBuffer->Bind();
        glViewport(0, 0, width, height);
        UseProgram();

        for(const TRenderInfo& info : objects.GetParticles())
        {
			info.particles->Draw(false);
        }

    }
    glPopDebugGroup();
}

void ParticlePass::UseProgram()
{
    if(!program)
    {
        GenerateProgram();
    }

    program->Use();
}

void ParticlePass::GenerateProgram()
{
	std::unique_ptr<Shader> vertex, fragment;

	vertex.reset(Shader::CreateVSFromFile("assets/shaders/particles.vs", 0, 0));

	bool ok = vertex->Compiled();

	if (ok)
	{

        fragment.reset(Shader::CreateFSFromFile("assets/shaders/particles.fs", 0, 0));

		ok = fragment->Compiled();
	}

	if (ok)
	{
		program = std::make_unique<Program>(vertex.get(), fragment.get(), "Particles");

		ok = program->Linked();
	}

	if (!ok)
	{
		program.release();
	}
}
