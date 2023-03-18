#include "Globals.h"

#include "LinePass.h"

#include "ComponentLine.h"

#include "OGL.h"
#include "OpenGL.h"
#include "RenderList.h"

LinePass::LinePass()
{

}

LinePass::~LinePass()
{

}

void LinePass::execute(const RenderList& objects)
{
    UseProgram();

    for(const TRenderInfo& info : objects.GetLines())
    {
        info.line->GetVAO()->Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }
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