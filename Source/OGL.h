#ifndef __OGL_H__
#define __OGL_H__

#include "Math.h"
#include <string>

class Texture2D
{
    uint texture    = 0;
    uint tex_target = 0;
public:
    Texture2D(const Texture2D& rhs) = delete;
    Texture2D& operator=(const Texture2D& rhs) = delete;

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

    static Texture2D* CreateDefaultRGBA(uint width, uint height, void* data = nullptr, bool mipmaps = false);

private:
    void DefaultInitialize(bool mipmaps);
};

class Framebuffer
{
    uint fbo = 0;
    uint attachments[16];
    uint attach_count = 0;
public:
    Framebuffer(const Framebuffer& rhs) = delete;
    Framebuffer& operator=(const Framebuffer& rhs) = delete;

    Framebuffer();
    ~Framebuffer();

    void Clear(uint width, uint height);
    void Unbind();
    void Bind();

    void AttachColor(Texture2D* texture, uint attachment = 0, uint mip_level = 0);
    void AttachDepthStencil(Texture2D* texture, uint attachment);
    void ReadColor(uint attachment, uint x, uint y, uint widht, uint height, uint format, unsigned* data);

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
    Buffer(const Buffer& rhs) = delete;
    Buffer& operator=(const Buffer& rhs) = delete;

    Buffer(uint type, uint usage, uint size, void* data);
    ~Buffer();

    void  Bind();

    uint  Id() const { return id;  }
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
    VertexArray(const VertexArray& rhs) = delete;
    VertexArray& operator=(const VertexArray& rhs) = delete;

    VertexArray(Buffer* vbo, Buffer* ibo, VertexAttrib attribs[], uint count);
    ~VertexArray();

    void Bind();
    void Unbind();

};

class Shader
{
    bool compiled  = false;
    unsigned id    = 0;
    unsigned type  = 0;

public:
    Shader(const Shader& rhs) = delete;
    Shader& operator=(const Shader& rhs) = delete;

    Shader(unsigned type, const char** source, unsigned count);
    Shader(unsigned type, const char* path, const char** defines = nullptr, unsigned count=0    );
    ~Shader();

    unsigned Id       () const {return id;}
    unsigned Type     () const {return type;}
    bool     Compiled () const {return compiled;}

    static Shader* CreateVSFromFile      (const char* path, const char** defines = nullptr, unsigned count = 0);
    static Shader* CreateFSFromFile      (const char* path, const char** defines = nullptr, unsigned count = 0);
    static Shader* CreateCompouteFromFile(const char* path, const char** defines = nullptr, unsigned count = 0);

private:

    void Init(unsigned type, const char** source, unsigned count, std::string& output);
};

class Program
{
    unsigned id = 0;
    bool     linked = false;
public:

    Program(const Program& rhs) = delete;
    Program& operator=(const Program& rhs) = delete;

    Program(const Shader* vs, const Shader* fs, unsigned count, const char* log_name = nullptr);
    Program(const Shader** shaders, unsigned count, const char* log_name = nullptr);
    ~Program();

    unsigned Id         () const { return id; } 
    bool     Linked     () const { return linked; }
    void     Use        ();
    void     Unuse      ();

    void     BindTexture(uint location, uint unit, const Texture2D* texture);

    void     BindUniform(uint location, int value);
    void     BindUniform(uint location, float value);
    void     BindUniform(uint location, const float2& value);
    void     BindUniform(uint location, const float3& value);
    void     BindUniform(uint location, const float4& value);
    void     BindUniform(uint location, const float2x2& value);
    void     BindUniform(uint location, const float3x3& value);
    void     BindUniform(uint location, const float4x4& value);
    void     BindUniform(uint location, int count, int* value);
    void     BindUniform(uint location, int count, float* value);
    void     BindUniform(uint location, int count, const float2* value);
    void     BindUniform(uint location, int count, const float3* value);
    void     BindUniform(uint location, int count, const float4* value);
    void     BindUniform(uint location, int count, const float2x2* value);
    void     BindUniform(uint location, int count, const float3x3* value);
    void     BindUniform(uint location, int count, const float4x4* value);

    void     BindSSBO   (unsigned binding, const Buffer* buffer);

private:

    void Init(const Shader** shaders, unsigned count, const char* log_name = nullptr);
};

#endif // __OGL_H__
