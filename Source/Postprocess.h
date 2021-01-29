#ifndef _POSTPRO_H_
#define _POSTPRO_H_


class Texture2D;
class Framebuffer;

class Postprocess
{
public:

    Postprocess();
    ~Postprocess();

    void Init();
    void Execute(Texture2D* screen, Framebuffer* fbo, unsigned width, unsigned height);

private:

    void GenerateBloomFBO(unsigned width, unsigned height);

private:
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

};

#endif /* _POSTPRO_H_ */
