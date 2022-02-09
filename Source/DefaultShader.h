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

	void Render(BatchManager* batch, const RenderList& objects, const Buffer* cameraUBO);

private:

	void UseProgram();
	bool GenerateProgram();
};

#endif /*_DEFAULT_SHADER_H_*/
