#pragma once

#include "Math.h"
#include <memory>

class Program;
class Texture;

class DepthRangePass
{
    std::unique_ptr<Program> programRed;
    std::unique_ptr<Program> programRedGreen;
    std::unique_ptr<Texture> texture0;
    std::unique_ptr<Texture> texture1;
    uint texWidth = 0;
    uint texHeight = 0;
    float2 minMax = float2::zero;

public:

    DepthRangePass();
    ~DepthRangePass();

    void execute(Texture* depthBuffer, uint width, uint height);
    const float2& getMinMaxDepth() const {return minMax;}

private:

    void generatePrograms();
    void createTextures(uint width, uint height);
};