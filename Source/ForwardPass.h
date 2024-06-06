#ifndef _FORWARD_PASS_H_
#define _FORWARD_PASS_H_

#include "RenderList.h"

#include "BatchDrawCommands.h"
#include <memory>

class Program;
class Framebuffer;
class ComponentCamera;

class ForwardPass
{
public:

	std::unique_ptr<Program> program;
    BatchDrawCommands opaqueCommands;
    BatchDrawCommands transparentCommands;

public:

	ForwardPass();
	~ForwardPass();

	void executeOpaque(ComponentCamera* culling, Framebuffer *target, uint width, uint height);
	void executeTransparent(ComponentCamera* culling, Framebuffer *target, uint width, uint height);

private:

	void UseProgram();
	bool GenerateProgram();
    void bindShadows();

};

#endif /*_FORWARD_PASS_H_*/
