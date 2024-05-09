#pragma once

#include <memory>
#include "OGL.h"
#include "Timer.h"

class Program;
class Framebuffer;
class VertexArray;
class GaussianBlur;
class DualKawaseBlur;

class VolumetricPass
{
    std::unique_ptr<Program>        program;
    std::unique_ptr<Program>        coneProgram;
    std::unique_ptr<Program>        applyProgram;
    std::unique_ptr<Program>        applyProgramNoBlur;
    std::unique_ptr<VertexArray>    vao;
    std::unique_ptr<Buffer>         parametersUBO;
    std::unique_ptr<Framebuffer>    frameBuffer;
    std::unique_ptr<Texture2D>      result;
    std::unique_ptr<DualKawaseBlur> kawase;


    uint fbWidth = 0;
    uint fbHeight = 0;

    float frame = 0.0f;
    Timer timer;

public:
    VolumetricPass();
    ~VolumetricPass();

    void execute(Framebuffer* target, uint width, uint height);

private:
    void useProgram();
    void useApplyProgram(bool blur);    
    void useConeProgram();
    void generateApplyProgram(bool blur);
    void resizeFrameBuffer(uint width, uint height);
};