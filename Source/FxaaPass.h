#pragma once

#include <memory>


class Program;
class Texture2D;
class Framebuffer;

class FxaaPass
{
    std::unique_ptr<Program> program;
    std::unique_ptr<Texture2D> output;
    std::unique_ptr<Framebuffer> frameBuffer; 
    uint fbWidth = 0;
    uint fbHeight = 0;

public:

    FxaaPass();
    ~FxaaPass();

    void execute(Texture2D* ldrOutput, unsigned width, unsigned height);

    const Texture2D* getOutput() const {return output.get();}

private:

    void generateFramebuffer(uint width, uint height);
    bool generateProgram();

};