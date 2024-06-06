#ifndef _GBUFFEREXPORT_PASS_H_
#define _GBUFFEREXPORT_PASS_H_

#include "BatchDrawCommands.h"
#include <memory>

class Framebuffer;
class RenderList;
class Texture2D;
class Buffer;
class Program;
class ComponentCamera;

class GBufferExportPass
{
    std::unique_ptr<Framebuffer> frameBuffer;

    std::unique_ptr<Texture2D>   albedoTex;
    std::unique_ptr<Texture2D>   specularTex;
    std::unique_ptr<Texture2D>   emissiveTex;
    std::unique_ptr<Texture2D>   positionTex;
    std::unique_ptr<Texture2D>   normalTex;
    std::unique_ptr<Texture2D>   depthTex;

    std::unique_ptr<Program>     program;
    std::unique_ptr<Program>     programCascade;

    BatchDrawCommands            drawCommands;


    uint                         fbWidth = 0;
    uint                         fbHeight = 0;

public:

    GBufferExportPass();
    ~GBufferExportPass();

    void execute(ComponentCamera* culling, uint width, uint height);

    Framebuffer* getFB() const {return frameBuffer.get(); }
    Texture2D*   getAlbedo() const {return albedoTex.get();}
    Texture2D*   getSpecular() const {return specularTex.get();}
    Texture2D*   getEmissive() const { return emissiveTex.get(); }
    Texture2D*   getPosition() const {return positionTex.get();}
    Texture2D*   getNormal() const {return normalTex.get();}
    Texture2D*   getDepth() const {return depthTex.get();}

    uint         getWidth() const {return fbWidth;}
    uint         getHeight() const {return fbHeight;}
    
private:

    void resizeFrameBuffer(uint width, uint height);
    void useProgram();
    bool generatePrograms();
};

#endif /* _GBUFFEREXPORT_PASS_H_ */