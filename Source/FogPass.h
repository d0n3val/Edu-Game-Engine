#pragma once

#include <memory>

class Program;
class Framebuffer;
class VertexArray;

class FogPass
{
    enum FogType
    {
        FOG_TYPE_DISTANCE = 0,
        FOG_TYPE_HEIGHT
    };
    std::unique_ptr<Program> program;
    std::unique_ptr<Program> distanceProg;
    std::unique_ptr<VertexArray> vao;

public:
    FogPass();

    void execute(Framebuffer* target, uint width, uint height);

private:
    void useProgram();
    bool generateProgram();
    void useDistanceProgram();
    bool generateDistanceProgram();
};