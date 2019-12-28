#ifndef __OGL_H__
#define __OGL_H__

class Texture2D
{
    uint texture = 0;
    uint target;
public:
	Texture2D();
    explicit Texture2D(uint target);
    ~Texture2D();

    void bind(uint uinit = 0);
    void unbind(uint uinit = 0);

    void initialize(uint width, uint height, uint internal_format, uint format, uint type, void* data, bool mipmaps);
    void set_data(uint width, uint height, uint mip_level, uint internal_format, uint format, uint type, void* data);

    void set_wrapping(uint wrap_s, uint wrap_t, uint wrap_r);
    void set_min_max_filer(uint min_filter, uint max_filter);

    void generate_mipmaps(uint base = 0, uint max = 1000);
};

class Framebuffer
{
    uint fbo = 0;
public:
    Framebuffer();
    ~Framebuffer();

    void unbind();
    void bind();

    // \todo: attach multiple color attachments (deferred)
    void attach_color_texture2D(uint texture, uint attachment, bool msaa, uint mip_level, bool draw, bool read);
    void attach_depth_stencil_texture(uint texture);
};


#endif // __OGL_H__
