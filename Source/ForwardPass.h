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

	void executeOpaque(const RenderList& objects, Framebuffer *target, uint width, uint height);
	void executeTransparent(const RenderList& objects, Framebuffer *target, uint width, uint height);

private:

	void UseProgram();
	bool GenerateProgram();
    void bindShadows();

};

#endif /*_FORWARD_PASS_H_*/
