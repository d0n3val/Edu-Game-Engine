#pragma once

#include <memory>
#include "OGL.h"
#include "Timer.h"

class Program;
class Framebuffer;
class VertexArray;
class GaussianBlur;
class DualKawaseBlur;

class FogPass
{
    enum FogType
    {
        FOG_TYPE_DISTANCE = 0,
        FOG_TYPE_HEIGHT,
        FOG_TYPE_RAYMARCHING
    };
    std::unique_ptr<Program> program;
    std::unique_ptr<Program> distanceProg;
    std::unique_ptr<Program> applyProg;
    std::unique_ptr<Program> applyProgNoBlur;
    std::unique_ptr<Program> rayMarchingProgram;
    std::unique_ptr<VertexArray> vao;
    std::unique_ptr<Buffer> ubo;
    std::unique_ptr<Buffer> parametersUBO;
    std::unique_ptr<Framebuffer> frameBuffer;
    std::unique_ptr<Texture2D>   result;
    std::unique_ptr<Texture2D>   rayMarchingResult;
    std::unique_ptr<GaussianBlur> blur;
    std::unique_ptr<DualKawaseBlur> kawase;
    std::unique_ptr<DualKawaseBlur> kawase2;

    uint fbWidth = 0;
    uint fbHeight = 0;

    float frame = 0.0f;
    Timer timer;

public:
    FogPass();
    ~FogPass();

    void execute(Framebuffer* target, uint width, uint height);

private:
    void useProgram();
    void useDistanceProgram();
    void useApplyProgram();
    void useApplyProgramNoBlur();
    void useRayMarchingProgram();

    Program* generateProgram(const char* name, const char* vertexPath, const char* fragmentPath, const char** defines, unsigned count);
    void resizeFrameBuffer(uint width, uint height);
};