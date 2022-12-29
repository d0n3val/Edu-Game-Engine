#pragma once

#include <memory>

class Program;
class Framebuffer;
class VertexArray;

class FogPass
{
    std::unique_ptr<Program> program;
    std::unique_ptr<VertexArray> vao;

public:
    FogPass();

    void execute(Framebuffer* target, uint width, uint height);

private:
    void useProgram();
    bool generateProgram();
};