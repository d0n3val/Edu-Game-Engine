#ifndef __OGL_H__
#define __OGL_H__

class Texture2D
{
    uint texture = 0;
    uint tex_target;
public:
    Texture2D(uint target, uint width, uint height, uint internal_format, uint format, uint type, void* data, bool mipmaps);
    Texture2D(uint target, uint samples, uint width, uint height, uint internal_format, bool fixed_samples);
    ~Texture2D();

    void bind(uint unit = 0) const;
    void unbind(uint unit = 0) const;


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

    void unbind() const;
    void bind() const;

    // \todo: attach multiple color attachments (deferred)
    // \todo: Blit frame buffers
    void attach_color(const Texture2D* texture, uint attachment = 0, uint mip_level = 0, bool draw = true, bool read = true);
    void attach_depth_stencil(const Texture2D* texture);

    uint id() const { return fbo; }
};


#endif // __OGL_H__
