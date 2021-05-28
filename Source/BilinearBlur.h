#ifndef _BILINEAR_BLUR_H_
#define _BILINEAR_BLUR_H_

#include <memory>

class Framebuffer;
class Texture2D;
class Program;

class BilinearBlur
{

    std::unique_ptr<Framebuffer> frameBuffer;
    std::unique_ptr<Texture2D>   result;
    std::unique_ptr<Program>     program;
    uint                         fbWidth = 0;
    uint                         fbHeight = 0;

public:

    BilinearBlur();

    void Execute(const Texture2D *input, const Texture2D* output, uint width, uint height);
};


#endif /* _BILINEAR_BLUR_H_ */