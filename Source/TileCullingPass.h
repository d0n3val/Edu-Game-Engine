#pragma once

#include "OGL.h"
#include <memory>

class TileCullingPass
{
    std::unique_ptr<Program> program;
    std::unique_ptr<TextureBuffer> pointListTex;
    std::unique_ptr<TextureBuffer> spotListTex;
    std::unique_ptr<Buffer> pointListBuffer;
    std::unique_ptr<Buffer> spotListBuffer;
    std::unique_ptr<TextureBuffer> dbgTex;
    std::unique_ptr<Buffer> dbgBuffer;
    uint bufferSize = 0;
public:
    TileCullingPass();
    ~TileCullingPass();

    void execute();

    const TextureBuffer* getPointLightList() const {return pointListTex.get();}
    const TextureBuffer* getSpotLightList() const {return spotListTex.get();}

private:

    void useProgram();
    void generateTextureBuffer(int tilesX, int tilesY);
};