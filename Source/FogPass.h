#pragma once

#include <memory>

class Program;
class Framebuffer;

class FogPass
{
    std::unique_ptr<Program> program;
public:
    FogPass();

    void execute(Framebuffer* target, uint width, uint height);

private:
    void useProgram();
    bool generateProgram();
};