#ifndef _FORWARD_PASS_H_
#define _FORWARD_PASS_H_

#include "RenderList.h"

#include <memory>

class Program;
class Framebuffer;

class ForwardPass
{
public:

	std::unique_ptr<Program> program;

public:

	ForwardPass();
	~ForwardPass();

	void executeOpaque(Framebuffer *target, uint width, uint height, const RenderList& objects);
	void executeTransparent(Framebuffer *target, uint width, uint height, const RenderList& objects);

private:

	void UseProgram();
	bool GenerateProgram();
};

#endif /*_FORWARD_PASS_H_*/
