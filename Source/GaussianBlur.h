#ifndef _BLUR_H_
#define _BLUR_H_

#include <memory>

class Framebuffer;
class Texture2D;
class Program;
class VertexArray;

class GaussianBlur
{

    std::unique_ptr<Framebuffer> frameBufferH;
    std::unique_ptr<Framebuffer> frameBufferV;
    std::unique_ptr<Texture2D>   result;
    std::unique_ptr<Program>     horizontal;
    std::unique_ptr<Program>     vertical;
    std::unique_ptr<VertexArray> vao;

    uint                         rWidth = 0;
    uint                         rHeight = 0;
    uint                         rInternal = 0;
    uint                         rFormat = 0;
    uint                         rType = 0;

public:

    GaussianBlur();

    void execute(const Texture2D *input, const Texture2D* output, uint internal_format, uint format, uint type, 
                 uint inMip, uint inWidth, uint inHeight, uint outMip, uint outWidth, uint outHeight);

private:
    void createResult(uint internal_format, uint format, uint type, uint width, uint height);
};


#endif /* _BLUR_H_ */