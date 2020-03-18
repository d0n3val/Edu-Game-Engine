#ifndef __OGL_H__
#define __OGL_H__

class Texture2D
{
    uint texture    = 0;
    uint tex_target = 0;
public:
    Texture2D(uint target, uint tex) : tex_target(target), texture(tex) {;}
    Texture2D(uint target, uint width, uint height, uint internal_format, uint format, uint type, void* data, bool mipmaps);
    Texture2D(uint target, uint samples, uint width, uint height, uint internal_format, bool fixed_samples);
    ~Texture2D();

    void Bind(uint unit = 0);
    void Unbind(uint unit = 0);


    void SetData(uint width, uint height, uint mip_level, uint internal_format, uint format, uint type, void* data);
    void SetWrapping(uint wrap_s, uint wrap_t, uint wrap_r);
    void SetMinMaxFiler(uint min_filter, uint max_filter);
    void GenerateMipmaps(uint base = 0, uint max = 1000);

    uint Id() const { return texture; }
    uint Target() const { return tex_target; }

private:
    void DefaultInitialize(bool mipmaps);
};

class Framebuffer
{
    uint fbo = 0;
public:
    Framebuffer();
    ~Framebuffer();

    void Unbind();
    void Bind();

    // \todo: attach multiple color attachments (deferred)
    void AttachColor(Texture2D* texture, uint attachment = 0, uint mip_level = 0, bool draw = true, bool read = true);
    void AttachDepthStencil(Texture2D* texture, uint attachment);

    void BlitTo(Framebuffer* target, uint src_x0, uint src_y0, uint src_x1, uint src_y1, uint dst_x0, uint dst_y0, 
                 uint dest_x1, uint dest_y1, uint flags, uint filter);

    uint Id() const { return fbo; }

    uint Check();
};

class Buffer
{
    uint   type = 0;
    uint   id   = 0;

public:

    Buffer(uint type, uint usage, uint size, void* data);
    ~Buffer();

    void  Bind();

    void  Unbind();
    void* Map(uint access);
    void* MapRange(uint access, uint offset, uint size);
    // todo: maps for 'special' buffers like uniform buffers or shader storage buffers
    void  Unmap();
    void  SetData(uint offset, uint size, void* data);

    static Buffer* CreateVBO(uint usage, uint size, void* data);
    static Buffer* CreateIBO(uint usage, uint size, void* data);
};

struct VertexAttrib
{
    uint index        = 0;
    uint num_elements = 0;
    uint type         = 0;
    bool normalize    = false;
    uint stride       = 0;
    uint offset       = 0;
};

class VertexArray
{

    uint id = 0;

public:

    VertexArray(Buffer* vbo, Buffer* ibo, VertexAttrib attribs[], uint count);
    ~VertexArray();

    void Bind();
    void Unbind();

};

#endif // __OGL_H__
