#pragma once

#include <memory>

class Program;
class Framebuffer;
class Buffer;

class DeferredResolveProxy
{
    std::unique_ptr<Program> program;
    std::unique_ptr<Program> debugProgram;
    std::unique_ptr<Buffer>  drawIdVBO;
    uint                     drawIdCount = 0;
public:

    void execute(Framebuffer* target, uint width, uint height);

private:
    void useProgram();
    bool generateProgram();
    void useDebug();
    bool generateDebug();
    void createDrawIdVBO();
};