#ifndef __OGL_H__
#define __OGL_H__

#include "Math.h"
#include <string>

class Texture
{
protected:

    uint tex_target = 0;
    uint texture = 0;

public:

    explicit Texture(uint target);
    Texture(uint target, uint text);
    ~Texture();

    void DefaultInitializeTexture(bool mipmaps);

    void SetWrapping(uint wrap_s, uint wrap_t, uint wrap_r);
    void SetMinMaxFiler(uint min_filter, uint max_filter);
    void GenerateMipmaps(uint base, uint max);

    void Bind(uint unit, uint uniform_location);
    void Bind(uint unit);
    void Unbind(uint unit);

    uint Id() const { return texture; }
    uint Target() const { return tex_target; }
};


class Texture2D : public Texture
{
public:

    Texture2D(const Texture2D& rhs) = delete;
    Texture2D& operator=(const Texture2D& rhs) = delete;

    Texture2D(uint target, uint tex);
    Texture2D(uint width, uint height, uint internal_format, uint format, uint type, void* data, bool mipmaps);
    Texture2D(uint samples, uint width, uint height, uint internal_format, bool fixed_samples);

    void SetData(uint width, uint height, uint mip_level, uint internal_format, uint format, uint type, void* data);
    void SetDefaultRGBAData(uint width, uint height, void* data);

    static Texture2D* CreateDefaultRGBA(uint width, uint height, void* data = nullptr, bool mipmaps = false);
};

class Texture2DArray : public Texture
{
    uint width   = 0;
    uint height  = 0;
    uint depth   = 0;

public:

    Texture2DArray(const Texture2DArray& rhs) = delete;
    Texture2DArray& operator=(const Texture2DArray& rhs) = delete;

    Texture2DArray(uint mip_levels, uint width, uint height, uint depth, uint internal_format);

    void SetSubData(uint mip_level, uint depth_index, uint format, uint type, void* data);
    void SetDefaultRGBASubData(uint mip_level, uint depth_index, void* data);

    static Texture2DArray* CreateDefaultRGBA(uint mip_levels, uint width, uint height, uint depth, bool convert_linear);
};

class TextureCube : public Texture
{
public:

    TextureCube(const TextureCube&) = delete;
    TextureCube& operator=(const TextureCube&) = delete;

    TextureCube();

    void SetData(uint face_index, uint mip_level, uint width, uint height, uint internal_format, uint format, uint type, void* data);
    void SetDefaultRGBAData(uint face_index, uint mip_level, uint width, uint height, void* data);
    void GetData(uint face_index, uint mip_level, uint format, uint type, void* data);
    void GetDefaultRGBAData(uint face_index, uint mip_level, void* data);
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

    void Unbind();
    void Bind();
  
    void ClearAttachments();
    void AttachColor(const Texture2D* texture, uint attachment = 0, uint mip_level = 0);
    void AttachColor(TextureCube* texture, uint face, uint attachment = 0, uint mip_level = 0);
    void AttachDepthStencil(Texture2D* texture, uint attachment);
    void ReadColor(uint attachment, uint x, uint y, uint widht, uint height, uint format, unsigned* data);

    void ClearColor(uint buffer, float value[4]);
    void ClearColor(uint buffer, int value[4]);
    void ClearColor(uint buffer, unsigned value[4]);
    void ClearDepth(float value);
    void ClearStencil(int value);

    void BlitTo(Framebuffer* target, uint src_x0, uint src_y0, 
                uint src_x1, uint src_y1, uint dst_x0, uint dst_y0, 
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

    Buffer(uint type, uint usage, uint size, const void* data);
    ~Buffer();

    void  Bind();
    void  BindToTargetIdx(uint index);

    uint  Id() const { return id;  }
    void  Unbind();
    void* Map(uint access);
    void* MapRange(uint access, uint offset, uint size);
    void  Unmap();
    void  SetData(uint offset, uint size, const void* data);

    static Buffer* CreateVBO(uint usage, uint size, const void* data);
    static Buffer* CreateIBO(uint usage, uint size, const void* data);
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

    Program(const Shader* vs, const Shader* fs, const char* log_name = nullptr);
    Program(const Shader** shaders, unsigned count, const char* log_name = nullptr);
    ~Program();

    unsigned Id         () const { return id; } 
    bool     Linked     () const { return linked; }
    void     Use        ();
    void     Unuse      ();

    void     BindTexture(uint location, uint unit, const Texture* texture);
    void     BindTextureFromName(const char* name, uint unit, const Texture* texture);

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

    void     BindUniformFromName(const char* name, int value);
    void     BindUniformFromName(const char* name, float value);
    void     BindUniformFromName(const char* name, const float2& value);
    void     BindUniformFromName(const char* name, const float3& value);
    void     BindUniformFromName(const char* name, const float4& value);
    void     BindUniformFromName(const char* name, const float2x2& value);
    void     BindUniformFromName(const char* name, const float3x3& value);
    void     BindUniformFromName(const char* name, const float4x4& value);
    void     BindUniformFromName(const char* name, int count, int* value);
    void     BindUniformFromName(const char* name, int count, float* value);
    void     BindUniformFromName(const char* name, int count, const float2* value);
    void     BindUniformFromName(const char* name, int count, const float3* value);
    void     BindUniformFromName(const char* name, int count, const float4* value);
    void     BindUniformFromName(const char* name, int count, const float2x2* value);
    void     BindUniformFromName(const char* name, int count, const float3x3* value);
    void     BindUniformFromName(const char* name, int count, const float4x4* value);

    void     BindUniformBlock(const char* name, uint blockIndex);

    void     BindSSBO   (unsigned binding, const Buffer* buffer);

private:

    void Init(const Shader** shaders, unsigned count, const char* log_name = nullptr);
};

#endif // __OGL_H__
