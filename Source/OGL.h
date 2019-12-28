#ifndef __OGL_H__
#define __OGL_H__

class Texture2D
{
    uint texture = 0;
    uint tex_target;
public:
    Texture2D(uint target, uint tex) : tex_target(target), texture(tex) {;}
    Texture2D(uint target, uint width, uint height, uint internal_format, uint format, uint type, void* data, bool mipmaps);
    Texture2D(uint target, uint samples, uint width, uint height, uint internal_format, bool fixed_samples);
    ~Texture2D();

    void bind(uint unit = 0);
    void unbind(uint unit = 0);


    void set_data(uint width, uint height, uint mip_level, uint internal_format, uint format, uint type, void* data);
    void set_wrapping(uint wrap_s, uint wrap_t, uint wrap_r);
    void set_min_max_filer(uint min_filter, uint max_filter);
    void generate_mipmaps(uint base = 0, uint max = 1000);

    uint id() const { return texture; }
    uint target() const { return tex_target; }

private:
    void default_initialize(bool mipmaps);
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
    void attach_color(Texture2D* texture, uint attachment = 0, uint mip_level = 0, bool draw = true, bool read = true);
    void attach_depth_stencil(Texture2D* texture);

    void blit_to(Framebuffer* target, uint src_x0, uint src_y0, uint src_x1, uint src_y1, uint dst_x0, uint dst_y0, 
                 uint dest_x1, uint dest_y1, uint flags, uint filter);

    uint id() const { return fbo; }
};


#endif // __OGL_H__
