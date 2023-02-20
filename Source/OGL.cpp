#include "Globals.h"
#include "OGL.h"
#include "OpenGL.h"

#include "Leaks.h"

Texture::Texture(uint target) : tex_target(target)
{
    glGenTextures(1, &texture);
}

Texture::Texture(uint target, uint text) : tex_target(target), texture(text)
{
}

Texture::~Texture()
{
    if(handle != 0)
    {
        glMakeTextureHandleNonResidentARB(handle);
    }

    glDeleteTextures(1, &texture);
}

void Texture::DefaultInitializeTexture(bool mipmaps)
{
    if(mipmaps)
    {
        glTexParameteri(tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(tex_target, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(tex_target, GL_TEXTURE_MAX_LEVEL, 1000);
        glTexParameteri(tex_target, GL_TEXTURE_MAX_ANISOTROPY, 1);
        glGenerateMipmap(tex_target);
    }
    else
    {
        glTexParameteri(tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glTexParameteri(tex_target, GL_TEXTURE_BASE_LEVEL, 0);
        //glTexParameteri(tex_target, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri(tex_target, GL_TEXTURE_MAX_ANISOTROPY, 1);
    }

    glTexParameteri(tex_target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(tex_target, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Texture::SetWrapping(uint wrap_s, uint wrap_t, uint wrap_r)
{
    glBindTexture(tex_target, texture);
    glTexParameteri(tex_target, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(tex_target, GL_TEXTURE_WRAP_T, wrap_t);
    glTexParameteri(tex_target, GL_TEXTURE_WRAP_R, wrap_r);
    glBindTexture(tex_target, 0);
}

void Texture::SetMinMaxFiler(uint min_filter, uint max_filter)
{
    glBindTexture(tex_target, texture);
    glTexParameteri(tex_target, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(tex_target, GL_TEXTURE_MAG_FILTER, max_filter);
    glBindTexture(tex_target, 0);
}

void Texture::GenerateMipmaps(uint base, uint max)
{
    glBindTexture(tex_target, texture);
    glTexParameteri(tex_target, GL_TEXTURE_BASE_LEVEL, base);
    glTexParameteri(tex_target, GL_TEXTURE_MAX_LEVEL, max);
    glGenerateMipmap(tex_target);
    glBindTexture(tex_target, 0);
}

void Texture::Bind(uint unit, uint uniform_location) const 
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(tex_target, texture);
    glUniform1i(uniform_location, unit);
}

void Texture::Bind(uint unit)  const 
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(tex_target, texture);
}

void Texture::Unbind(uint unit) const 
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(tex_target, 0);
}

uint64_t Texture::GetBindlessHandle() 
{
    if(handle == 0)
    {
        handle = glGetTextureHandleARB(texture);
        glMakeTextureHandleResidentARB(handle);
    }

    return handle;
}

void Texture::MakeBindlessResident()
{
    glMakeTextureHandleResidentARB(GetBindlessHandle());
}

void Texture::MakeBindlessNonResident()
{
    glMakeTextureHandleNonResidentARB(GetBindlessHandle());
}

Texture2D::Texture2D(uint target, uint tex) : Texture(target, tex)
{
}

Texture2D::Texture2D(uint samples, uint width, uint height, uint internal_format, bool fixed_samples) : Texture(GL_TEXTURE_2D_MULTISAMPLE)
{
    glBindTexture(tex_target, texture);
    glTexImage2DMultisample(tex_target, samples, internal_format, width, height, fixed_samples ? GL_TRUE : GL_FALSE);    
    glBindTexture(tex_target, 0);
}

Texture2D::Texture2D(uint width, uint height, uint internal_format, uint format, uint type, void* data, bool mipmaps) : Texture(GL_TEXTURE_2D)
{
    glBindTexture(tex_target, texture);

    glTexImage2D(tex_target, 0, internal_format, width, height, 0, format, type, data);

    DefaultInitializeTexture(mipmaps);

    glBindTexture(tex_target, 0);
}

Texture2D::Texture2D(uint width, uint height, uint internalFormat, uint compressedSize, void* compressedData, bool mipmaps) : Texture(GL_TEXTURE_2D)
{
    glBindTexture(tex_target, texture);

    glCompressedTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, compressedSize, compressedData);
    
    DefaultInitializeTexture(mipmaps);

    glBindTexture(tex_target, 0);
}

void Texture2D::SetCompressedData(uint width, uint height, uint internalFormat, uint compressedSize, void *compressedData, bool mipMaps)
{
    glBindTexture(tex_target, texture);
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, compressedSize, compressedData);
    glBindTexture(tex_target, 0);
}

void Texture2D::SetData(uint width, uint height, uint mip_level, uint internal_format, uint format, uint type, void *data)
{
    glBindTexture(tex_target, texture);
    glTexImage2D(tex_target, mip_level, internal_format, width, height, 0, format, type, data);
    glBindTexture(tex_target, 0);
}

void Texture2D::SetDefaultRGBAData(uint width, uint height, void* data)
{
    SetData(width, height, 0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

Texture2D* Texture2D::CreateDefaultRGBA(uint width, uint height, void* data, bool mipmaps)
{
    return new Texture2D(width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, data, mipmaps);
}

Texture2DArray::Texture2DArray(uint mipLevels, uint _width, uint _height, uint _depth, uint internal_format): Texture(GL_TEXTURE_2D_ARRAY)
{
    width  = _width;
    height = _height;
    depth  = _depth;

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipLevels, internal_format, width, height, depth);

    DefaultInitializeTexture(false);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void Texture2DArray::SetSubData(uint mip_level, uint depth_index, uint format, uint type, void* data)
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, mip_level, 0, 0, depth_index, width, height, 1, format, type, data);
	
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void Texture2DArray::SetCompressedSubData(uint mip_level, uint depth_index, uint format, uint imageSize, void *data)
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, mip_level, 0, 0, depth_index, width, height, 1, format, imageSize, data);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void Texture2DArray::SetDefaultRGBASubData(uint mip_level, uint depth_index, void* data)
{
    SetSubData(mip_level, depth_index, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

Texture2DArray* Texture2DArray::CreateDefaultRGBA(uint mipLevels, uint width, uint height, uint depth, bool convert_linear)
{
    return new Texture2DArray(mipLevels, width, height, depth, convert_linear ? GL_SRGB8_ALPHA8 : GL_RGB8);
}

void Texture2DArray::GenerateMipmaps()
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(tex_target, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(tex_target, GL_TEXTURE_MAX_LEVEL, 1000);
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

TextureCube::TextureCube() : Texture(GL_TEXTURE_CUBE_MAP)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void TextureCube::GetData(uint face_index, uint mip_level, uint format, uint type, void* data)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X+face_index, mip_level, format, type, data);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void TextureCube::SetData(uint face_index, uint mip_level, uint width, uint height, uint internal_format, uint format, uint type, void* data)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_index, mip_level, internal_format, width, height, 0, format, type, data);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void TextureCube::SetDefaultRGBAData(uint face_index, uint mip_level, uint width, uint height, void* data)
{
    SetData(face_index, mip_level, width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void TextureCube::GetDefaultRGBAData(uint face_index, uint mip_level, void* data)
{
    GetData(face_index, mip_level, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

TextureBuffer::TextureBuffer(Buffer *buffer, uint format)
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_BUFFER, id);
    glTexBuffer(GL_TEXTURE_BUFFER, format, buffer->Id());
    glBindTexture(GL_TEXTURE_BUFFER, 0);
}

void TextureBuffer::Bind(uint unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_BUFFER, id);
}

void TextureBuffer::Unbind(uint unit) const
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(GL_TEXTURE_BUFFER, 0);
}

Framebuffer::Framebuffer()
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glDrawBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &fbo);
}

/*
void Framebuffer::Clear(uint width, uint height)
{
    Bind();

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
*/

void Framebuffer::Bind() 
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::Unbind() 
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::AttachColor(const Texture2D* texture, uint attachment, uint mip_level) 
{
    Bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachment, texture->Target(), texture->Id(), mip_level);
    attachments[attach_count++] = GL_COLOR_ATTACHMENT0+attachment;

    glDrawBuffers(attach_count, attachments);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);

    Unbind();
}

void Framebuffer::ClearAttachments()
{
    Bind();
    attach_count = 0;
    glDrawBuffers(0, 0);
    Unbind();
}

void Framebuffer::AttachColor(const TextureCube* texture, uint face, uint attachment, uint mip_level)
{
    Bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texture->Id(), mip_level);
    attachments[attach_count++] = GL_COLOR_ATTACHMENT0+attachment;

    glDrawBuffers(attach_count, attachments);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);

    Unbind();
}

void Framebuffer::AttachDepthStencil(const Texture2D* texture, uint attachment) 
{
    Bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texture->Target(), texture->Id(), 0);
    Unbind();
}

void Framebuffer::ReadColor(uint attachment, uint x, uint y, uint width, uint height, uint format, unsigned* data)
{
    Bind();
    glReadBuffer(GL_COLOR_ATTACHMENT0+attachment);
    glReadPixels(x, y, width, height, format, GL_UNSIGNED_BYTE, data);

	/*	invert the image	*/
	for( unsigned j = 0; j*2 < height; ++j )
	{
		int index1 = j * width ;
		int index2 = (height - 1 - j) * width ;
		for(unsigned i = width; i > 0; --i )
		{
			unsigned temp = data[index1];
            data[index1] = data[index2];
			data[index2] = temp;
			++index1;
			++index2;
		}
	}

    Unbind();
}

void Framebuffer::ClearColor(uint buffer, float value[4])
{
    Bind();
    glClearBufferfv(GL_COLOR, buffer, value);
    Unbind();
}

void Framebuffer::ClearColor(uint buffer, int value[4])
{
    Bind();
    glClearBufferiv(GL_COLOR, buffer, value);
    Unbind();
}

void Framebuffer::ClearColor(uint buffer, unsigned value[4])
{
    Bind();
    glClearBufferuiv(GL_COLOR, buffer, value);
    Unbind();
}

void Framebuffer::ClearDepth(float value)
{
    Bind();
    glClearBufferfv(GL_DEPTH, 0, &value);
    Unbind();
}

void Framebuffer::ClearStencil(int value)
{
    Bind();
    glClearBufferiv(GL_STENCIL, 0, &value);
    Unbind();
}

void Framebuffer::BlitTo(Framebuffer* target, uint src_x0, uint src_y0, uint src_x1, uint src_y1, uint dst_x0, uint dst_y0, 
                          uint dst_x1, uint dst_y1, uint flags, uint filter) 
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->Id());
    glBlitFramebuffer(src_x0, src_y0, src_x1, src_y1, dst_x0, dst_y0, dst_x1, dst_y1, flags, filter);
	Unbind();
}

uint Framebuffer::Check()
{
	Bind();
    uint res = uint(glCheckFramebufferStatus(GL_FRAMEBUFFER));
	Unbind();

	return res;
}

Buffer::Buffer(uint tp, uint flags, size_t size, const void* data, bool storage)
{
    type = tp;

    glGenBuffers(1, &id);

    glBindBuffer(type, id);
    if (storage)
    {
        glBufferStorage(type, size, data, flags);
    }
    else
    {
        glBufferData(type, size, data, flags);
    }
    glBindBuffer(type, 0);
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &id);
}

void Buffer::Bind() const
{
    glBindBuffer(type, id);
}

void Buffer::BindToPoint(uint index) const 
{
    assert(type == GL_ATOMIC_COUNTER_BUFFER || type == GL_TRANSFORM_FEEDBACK_BUFFER ||
           type == GL_UNIFORM_BUFFER || type ==  GL_SHADER_STORAGE_BUFFER);

    glBindBufferBase(type, index, id);
}

void Buffer::Unbind() const
{
    glBindBuffer(type, 0);
}

void* Buffer::Map(uint access)
{
    glBindBuffer(type, id);
    void* ptr = glMapBuffer(type, access);
    glBindBuffer(type, 0);

    return ptr;
}

void* Buffer::MapRange(uint access, uint offset, uint size)
{
    glBindBuffer(type, id);
    void* ptr = glMapBufferRange(type, offset, size, access);
    glBindBuffer(type, 0);

    return ptr;
}

void Buffer::Unmap()
{
    glBindBuffer(type, id);
    glUnmapBuffer(type);
    glBindBuffer(type, 0);
}

void Buffer::SetData(uint offset, uint size, const void* data)
{
    glBindBuffer(type, id);
    glBufferSubData(type, offset, size, data);
    glBindBuffer(type, 0);
}

void Buffer::InvalidateData()
{
    glInvalidateBufferData(id);
}

Buffer* Buffer::CreateVBO(uint usage, uint size, const void* data)
{
    return new Buffer(GL_ARRAY_BUFFER, usage, size, data);
}

Buffer* Buffer::CreateIBO(uint usage, uint size, const void* data)
{
    return new Buffer(GL_ELEMENT_ARRAY_BUFFER, usage, size, data);
}

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &id);
    glBindVertexArray(id);
    glBindVertexArray(0);
}

VertexArray::VertexArray(Buffer* vbo, Buffer* ibo, VertexAttrib attribs[], uint count)
{
    glGenVertexArrays(1, &id);
    glBindVertexArray(id);
    vbo->Bind();

    for (uint32_t i = 0; i < count; i++)
    {
        glEnableVertexAttribArray(attribs[i].index);

        if (attribs[i].type == GL_INT || attribs[i].type == GL_UNSIGNED_INT)
        {
            glVertexAttribIPointer(attribs[i].index, attribs[i].num_elements, attribs[i].type, attribs[i].stride, (const void*)size_t(attribs[i].offset));
        }
        else
        {
            glVertexAttribPointer(attribs[i].index, attribs[i].num_elements, attribs[i].type, attribs[i].normalize, attribs[i].stride, (const void*)size_t(attribs[i].offset));
        }

        glVertexAttribDivisor(attribs[i].index, attribs[i].divisor);
    }

    if (ibo)
    {
        ibo->Bind();
    }

    glBindVertexArray(0);
}

VertexArray::VertexArray(Buffer* vbo[], Buffer* ibo, VertexAttrib attribs[], uint count)
{
    glGenVertexArrays(1, &id);
    glBindVertexArray(id);

    for (uint32_t i = 0; i < count; i++)
    {
        if (vbo[i])
        {
            vbo[i]->Bind();
            glEnableVertexAttribArray(attribs[i].index);

            if (attribs[i].type == GL_INT || attribs[i].type == GL_UNSIGNED_INT)
            {
                glVertexAttribIPointer(attribs[i].index, attribs[i].num_elements, attribs[i].type, attribs[i].stride, (const void*)size_t(attribs[i].offset));
            }
            else
            {
                glVertexAttribPointer(attribs[i].index, attribs[i].num_elements, attribs[i].type, attribs[i].normalize, attribs[i].stride, (const void*)size_t(attribs[i].offset));
            }

            glVertexAttribDivisor(attribs[i].index, attribs[i].divisor);
        }
    }

    if (ibo)
    {
        ibo->Bind();
    }

    glBindVertexArray(0);
}


VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &id);
}

void VertexArray::Bind()
{
    glBindVertexArray(id);
}

void VertexArray::Unbind()
{
    glBindVertexArray(0);
}

Shader::Shader(unsigned type, const char** source, unsigned count)
{
    std::string error;
    Init(type, source, count, error);

    if(!compiled)
    {
        LOG("Error compiling shader: %s", error.c_str());
    }
}

Shader::Shader(unsigned type, const char* path, const char** defines, unsigned count)
{
    // Read whole file
    char* data = nullptr;

	FILE* file = 0;

	fopen_s(&file, path, "rb");

    std::string error;

	if(file)
	{
		fseek(file, 0, SEEK_END);
		int size = ftell(file);
		rewind(file);
		data = (char*)malloc(size + 1);

		fread(data, 1, size, file);
		data[size] = 0;

		fclose(file);

        // inject defines
        if (count > 0)
        {
            // assume first line is #version and must remain being first line

            const char** final_data = (const char**)malloc(sizeof(char*) * ((count+1)*2 + 1));
            char* save_ptr = nullptr;
            char* version = strtok_s(data, "\n", &save_ptr);
            final_data[0] = version;
            final_data[1] = "\n";
            for (unsigned i = 0; i < count; ++i)
            {
                final_data[i*2 + 2] = defines[i];
                final_data[i*2 + 3] = "\n";
            }

            if (version != nullptr)
            {
                final_data[(count+1)*2 ] = strtok_s(nullptr, "", &save_ptr);

                Init(type, final_data, (count+1)*2 + 1, error);
            }
            else
            {
                Init(type, final_data, count*2 + 1, error);
            }


            free(data);
            free(final_data);
        }
        else
        {
            Init(type, (const char**)&data, 1, error);
            free(data);
        }
	}

    if(!compiled)
    {
        LOG("Error compiling shader %s: %s", path, error.c_str());
    }
}

void Shader::Init(unsigned type, const char** source, unsigned count, std::string& output)
{
    id = glCreateShader(type);

    GLint  success;
    GLchar log[512];

    glShaderSource(id, count, source, nullptr);

    glCompileShader(id);
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    compiled = success == GL_TRUE;

    if (!compiled)
    {
        glGetShaderInfoLog(id, 512, NULL, log);
        output = log;
    }
}

Shader* Shader::CreateVSFromFile(const char* path, const char** defines, unsigned count) 
{ 
    return new Shader(GL_VERTEX_SHADER, path, defines, count); 
}

Shader* Shader::CreateFSFromFile(const char* path, const char** defines, unsigned count) 
{ 
    return new Shader(GL_FRAGMENT_SHADER, path, defines, count); 
}

Shader* Shader::CreateCompouteFromFile(const char* path, const char** defines, unsigned count) 
{ 
    return new Shader(GL_COMPUTE_SHADER, path, defines, count); 
}


Shader::~Shader()
{
    glDeleteShader(id);
}

Program::Program(const Shader* vs, const Shader* fs, const char* log_name)
{
    const Shader* shader_list[] = { vs, fs };
    Init(shader_list, 2, log_name);
}

Program::Program(const Shader** shaders, unsigned count, const char* log_name)
{
    Init(shaders, count, log_name);
}

Program::Program(const Shader* shader)
{
    Init(&shader, 1, nullptr);
}

void Program::Init(const Shader** shaders, unsigned count, const char* log_name)
{
    id = glCreateProgram();

    for (unsigned i = 0; i < count; ++i)
    {
        glAttachShader(id, shaders[i]->Id());
    }

    glLinkProgram(id);

    int success;
    char  log[512];

    glGetProgramiv(id, GL_LINK_STATUS, &success);

    linked = success == GL_TRUE;

    if (!linked)
    {
        glGetProgramInfoLog(id, 512, NULL, log);
        LOG("Error linking program '%s' : %s",  log_name != nullptr ? log_name : "", log);
    }
}

Program::~Program()
{
    glDeleteProgram(id);
}

void Program::Use()
{
    glUseProgram(id);
}

void Program::Unuse()
{
    glUseProgram(0); 
}

void Program::BindTexture(uint location, uint unit, const Texture *texture)
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(texture->Target(), texture->Id());
    glUniform1i(location, unit);        
}

void Program::BindTextureFromName(const char* name, uint unit, const Texture* texture)
{
    BindTexture(glGetUniformLocation(id, name), unit, texture);
}

int Program::GetLocation(const char *name)
{
    return glGetUniformLocation(id, name);
}

void Program::BindUniform(int location, int value)
{
    glUniform1i(location, value);        
}

void Program::BindUniform(int location, float value)
{
    glUniform1f(location, value);        
}

void Program::BindUniform(int location, const float2& value)
{
    glUniform2f(location, value.x, value.y);
}

void Program::BindUniform(int location, const float3& value)
{
    glUniform3f(location, value.x, value.y, value.z);
}

void Program::BindUniform(int location, const float4& value)
{
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Program::BindUniform(int location, const float2x2& value)
{
    glUniformMatrix2fv(location, 1, GL_TRUE, reinterpret_cast<const float*>(&value));
}

void Program::BindUniform(int location, const float3x3& value)
{
    glUniformMatrix3fv(location, 1, GL_TRUE, reinterpret_cast<const float*>(&value));
}

void Program::BindUniform(int location, const float4x4& value)
{
    glUniformMatrix4fv(location, 1, GL_TRUE, reinterpret_cast<const float*>(&value));
}

void Program::BindUniform(int location, int count, int* value)
{
    glUniform1iv(location, count, value);
}

void Program::BindUniform(int location, int count, float* value)
{
    glUniform1fv(location, count, value);
}

void Program::BindUniform(int location, int count, const float2* value)
{
    glUniform2fv(location, count, reinterpret_cast<const float*>(value));
}

void Program::BindUniform(int location, int count, const float3* value)
{
    glUniform3fv(location, count, reinterpret_cast<const float*>(value));
}

void Program::BindUniform(int location, int count, const float4* value)
{
    glUniform4fv(location, count, reinterpret_cast<const float*>(value));
}

void Program::BindUniform(int location, int count, const float2x2* value)
{
    glUniformMatrix2fv(location, count, GL_TRUE, reinterpret_cast<const float*>(&value));
}

void Program::BindUniform(int location, int count, const float3x3* value)
{
    glUniformMatrix3fv(location, count, GL_TRUE, reinterpret_cast<const float*>(&value));
}

void Program::BindUniform(int location, int count, const float4x4* value)
{
    glUniformMatrix4fv(location, count, GL_TRUE, reinterpret_cast<const float*>(&value));
}

void Program::BindUniformFromName(const char* name, int value)
{
    BindUniform(glGetUniformLocation(id, name), value);
}

void Program::BindUniformFromName(const char* name, float value)
{
    BindUniform(glGetUniformLocation(id, name), value);
}

void Program::BindUniformFromName(const char* name, const float2& value)
{
    BindUniform(glGetUniformLocation(id, name), value);
}

void Program::BindUniformFromName(const char* name, const float3& value)
{
    BindUniform(glGetUniformLocation(id, name), value);
}

void Program::BindUniformFromName(const char* name, const float4& value)
{
    BindUniform(glGetUniformLocation(id, name), value);
}

void Program::BindUniformFromName(const char* name, const float2x2& value)
{
    BindUniform(glGetUniformLocation(id, name), value);
}

void Program::BindUniformFromName(const char* name, const float3x3& value)
{
    BindUniform(glGetUniformLocation(id, name), value);
}

void Program::BindUniformFromName(const char* name, const float4x4& value)
{
    BindUniform(glGetUniformLocation(id, name), value);
}

void Program::BindUniformFromName(const char* name, int count, int* value)
{
    BindUniform(glGetUniformLocation(id, name), count, value);
}

void Program::BindUniformFromName(const char* name, int count, float* value)
{
    BindUniform(glGetUniformLocation(id, name), count, value);
}

void Program::BindUniformFromName(const char* name, int count, const float2* value)
{
    BindUniform(glGetUniformLocation(id, name), count, value);
}

void Program::BindUniformFromName(const char* name, int count, const float3* value)
{
    BindUniform(glGetUniformLocation(id, name), count, value);
}

void Program::BindUniformFromName(const char* name, int count, const float4* value)
{
    BindUniform(glGetUniformLocation(id, name), count, value);
}

void Program::BindUniformFromName(const char* name, int count, const float2x2* value)
{
    BindUniform(glGetUniformLocation(id, name), count, value);
}

void Program::BindUniformFromName(const char* name, int count, const float3x3* value)
{
    BindUniform(glGetUniformLocation(id, name), count, value);
}

void Program::BindUniformFromName(const char* name, int count, const float4x4* value)
{
    BindUniform(glGetUniformLocation(id, name), count, value);
}

void Program::BindUniformBlock(const char* name, uint blockIndex)
{
    int index = glGetUniformBlockIndex(id, name);
    if (index != GL_INVALID_INDEX)
    {
        glUniformBlockBinding(id, index, blockIndex);
    }
}

void Program::BindSSBO(unsigned binding, const Buffer* buffer)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, buffer->Id());
}

void Program::BindSSBO(unsigned binding, const Buffer* buffer, uint offset, uint size)
{
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, binding, buffer->Id(), offset, size);
}