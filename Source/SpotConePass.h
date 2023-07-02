#pragma once

#include "RenderList.h"

#include <memory>

class Framebuffer;
class Program;

class SpotConePass
{
	std::unique_ptr<Program> program;
public:

    SpotConePass();
    ~SpotConePass();

    void execute(const RenderList& objects, Framebuffer *target, uint width, uint height);

private:
    void useProgram();
    bool generateProgram();
};

