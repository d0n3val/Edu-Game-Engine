#ifndef _POSTPRO_H_
#define _POSTPRO_H_

#include "Timer.h"
#include<memory>

class Program;
class Texture2D;
class Framebuffer;
class DualKawaseBlur;

class Postprocess
{
    uint post_vbo         = 0;
    uint post_vao         = 0;
    uint bloom_fbo        = 0;
    uint bloom_tex        = 0;
    uint color_tex        = 0;
    uint bloom_blur_fbo_0 = 0;
    uint bloom_blur_tex_0 = 0;
    uint bloom_blur_fbo_1 = 0;
    uint bloom_blur_tex_1 = 0;
    uint bloom_width      = 0;
    uint bloom_height     = 0;
    std::unique_ptr<DualKawaseBlur> kawase;

    float frame = 0.0f;
    Timer timer;
public:

    Postprocess();
    ~Postprocess();

    void Init();
    void Execute(const Texture2D* screen, const Texture2D* depth, Framebuffer* fbo, unsigned width, unsigned height);

private:

    void GenerateBloomFBO(unsigned width, unsigned height);

};

#endif /* _POSTPRO_H_ */
