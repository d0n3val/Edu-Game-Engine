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
    mutable float2 minMax = float2::zero;
    mutable Texture* result = nullptr;

public:

    DepthRangePass();
    ~DepthRangePass();

    void execute(Texture* depthBuffer, uint width, uint height);
    const float2& getMinMaxDepth() const { if (result!=nullptr) updateMinMax();  return minMax; }

private:

    void updateMinMax() const;
    void generatePrograms();
    void createTextures(uint width, uint height);
};