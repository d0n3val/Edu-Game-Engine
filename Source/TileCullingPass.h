#pragma once

#include "OGL.h"
#include <memory>

class TileCullingPass
{
    std::unique_ptr<Program> program;
    std::unique_ptr<TextureBuffer> pointListTex;
    std::unique_ptr<TextureBuffer> spotListTex;
    std::unique_ptr<TextureBuffer> volSpotListTex;
    std::unique_ptr<Buffer> pointListBuffer;
    std::unique_ptr<Buffer> spotListBuffer;
    std::unique_ptr<Buffer> volSpotListBuffer;
    std::unique_ptr<TextureBuffer> dbgTex;
    std::unique_ptr<Buffer> dbgBuffer;
    uint pointSize = 0;
    uint spotSize = 0;
public:
    TileCullingPass();
    ~TileCullingPass();

    void execute();

    const TextureBuffer* getPointLightList() const {return pointListTex.get();}
    const TextureBuffer* getSpotLightList() const {return spotListTex.get();}
    const TextureBuffer* getVolSpotLightList() const {return volSpotListTex.get();}

private:

    void useProgram();
    void generateTextureBuffer(int tilesX, int tilesY, int numPoints, int numSpots);
};