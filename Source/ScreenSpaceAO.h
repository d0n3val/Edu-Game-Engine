#ifndef _SCREEN_SPACE_AO_H_
#define _SCREEN_SPACE_AO_H_

#include <memory>

class Framebuffer;
class Texture2D;
class Buffer;
class Program;
class GaussianBlur;

class ScreenSpaceAO
{
    std::unique_ptr<Framebuffer>  frameBuffer;
    std::unique_ptr<Texture2D>    result;
    std::unique_ptr<Texture2D>    blurred;
    std::unique_ptr<Buffer>       kernel;
    std::unique_ptr<Program>      program;
    std::unique_ptr<GaussianBlur> blur;

    uint                         fbWidth = 0;
    uint                         fbHeight = 0;

public:

    ScreenSpaceAO();
    ~ScreenSpaceAO();

    void execute(uint width, uint height);

    const Texture2D* getResult() const;

private:
    void resizeFrameBuffer(uint width, uint height);
    void generateKernelUBO();
};


#endif /* _SCREEN_SPACE_AO_H_ */