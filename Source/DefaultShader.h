#ifndef _DEFAULT_SHADER_H_
#define _DEFAULT_SHADER_H_

#include "RenderList.h"

#include <memory>

class Program;
class BatchManager;
class Buffer;

class DefaultShader
{
public:

	std::unique_ptr<Program> program;

public:

	DefaultShader();
	~DefaultShader();

	void Render(const RenderList& objects);

private:

	void UseProgram();
	bool GenerateProgram();
};

#endif /*_DEFAULT_SHADER_H_*/
