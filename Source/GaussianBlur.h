#ifndef _BLUR_H_
#define _BLUR_H_

#include <memory>

class Framebuffer;
class Texture2D;
class Program;

class GaussianBlur
{

    std::unique_ptr<Framebuffer> frameBuffer;
    std::unique_ptr<Texture2D>   result;
    std::unique_ptr<Program>     horizontal;
    std::unique_ptr<Program>     vertical;
    uint                         fbWidth = 0;
    uint                         fbHeight = 0;

public:

    GaussianBlur();

    void Execute(const Texture2D *input, const Texture2D* output, uint width, uint height);
};


#endif /* _BLUR_H_ */