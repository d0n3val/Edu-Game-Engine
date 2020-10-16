#include "Globals.h"
#include "OGL.h"
#include "OpenGL.h"

#include "Leaks.h"

void DefaultInitializeTexture(uint tex_target, bool mipmaps)
{
    if(mipmaps)
    {
        glTexParameteri(tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(tex_target, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(tex_target, GL_TEXTURE_MAX_LEVEL, 1000);
        glGenerateMipmap(tex_target);
    }
    else
    {
        glTexParameteri(tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(tex_target, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(tex_target, GL_TEXTURE_MAX_LEVEL, 0);
    }
}

Texture2D::Texture2D(uint target, uint samples, uint width, uint height, uint internal_format, bool fixed_samples) : tex_target(target)
{
	glGenTextures(1, &texture);
    glBindTexture(tex_target, texture);

    glTexImage2DMultisample(tex_target, samples, internal_format, width, height, fixed_samples ? GL_TRUE : GL_FALSE);

    glBindTexture(tex_target, 0);
}

Texture2D::Texture2D(uint target, uint width, uint height, uint internal_format, uint format, uint type, void* data, bool mipmaps) : tex_target(target)
{
	glGenTextures(1, &texture);
    glBindTexture(tex_target, texture);

    glTexImage2D(tex_target, 0, internal_format, width, height, 0, format, type, data);
    DefaultInitializeTexture(tex_target, mipmaps);

    glBindTexture(tex_target, 0);
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &texture);
}

void Texture2D::Bind(uint unit, uint uniform_location)
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(tex_target, texture);
    glUniform1i(uniform_location, unit);
}

void Texture2D::Bind(uint unit) 
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(tex_target, texture);
}

void Texture2D::Unbind(uint unit) 
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(tex_target, 0);
}

void Texture2D::SetData(uint width, uint height, uint mip_level, uint internal_format, uint format, uint type, void* data)
{
    glBindTexture(tex_target, texture);
    glTexImage2D(tex_target, mip_level, internal_format, width, height, 0, format, type, data);
    glBindTexture(tex_target, 0);
}

void Texture2D::SetDefaultRGBAData(uint width, uint height, void* data)
{
    SetData(width, height, 0, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void Texture2D::SetWrapping(uint wrap_s, uint wrap_t, uint wrap_r)
{
    glBindTexture(tex_target, texture);
    glTexParameteri(tex_target, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(tex_target, GL_TEXTURE_WRAP_T, wrap_t);
    glTexParameteri(tex_target, GL_TEXTURE_WRAP_R, wrap_r);
    glBindTexture(tex_target, 0);
}

void Texture2D::SetMinMaxFiler(uint min_filter, uint max_filter)
{
    glBindTexture(tex_target, texture);
    glTexParameteri(tex_target, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(tex_target, GL_TEXTURE_MAG_FILTER, max_filter);
    glBindTexture(tex_target, 0);
}

void Texture2D::GenerateMipmaps(uint base, uint max)
{
    glBindTexture(tex_target, texture);
	glTexParameteri(tex_target, GL_TEXTURE_BASE_LEVEL, base);
	glTexParameteri(tex_target, GL_TEXTURE_MAX_LEVEL, max);
	glGenerateMipmap(tex_target);
    glBindTexture(tex_target, 0);
}

Texture2D* Texture2D::CreateDefaultRGBA(uint width, uint height, void* data, bool mipmaps)
{
    return new Texture2D(GL_TEXTURE_2D, width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, data, mipmaps);
}

Texture2DArray::Texture2DArray(uint mip_levels, uint _width, uint _height, uint _depth, uint internal_format)
{
    width  = _width;
    height = _height;
    depth  = _depth;

	glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, mip_levels, internal_format, width, height, depth);

    DefaultInitializeTexture(GL_TEXTURE_2D_ARRAY, mip_levels > 1);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

Texture2DArray::~Texture2DArray()
{
    glDeleteTextures(1, &texture);
}

void Texture2DArray::Bind(uint unit, uint uniform_location)
{
	glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glUniform1i(uniform_location, unit);
}

void Texture2DArray::Bind(uint unit /*= 0*/)
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
}

void Texture2DArray::Unbind(uint unit /*= 0*/)
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void Texture2DArray::SetSubData(uint mip_level, uint depth_index, uint format, uint type, void* data)
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, mip_level, 0, 0, depth_index, width, height, 1, format, type, data);
	
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void Texture2DArray::SetDefaultRGBASubData(uint mip_level, uint depth_index, void* data)
{
    SetSubData(mip_level, depth_index, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void Texture2DArray::GenerateMipmaps(uint base /*= 0*/, uint max /*= 1000*/)
{
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, base);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, max);
	glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

Texture2DArray* Texture2DArray::CreateDefaultRGBA(uint mip_levels, uint width, uint height, uint depth, bool convert_linear)
{
    return new Texture2DArray(mip_levels, width, height, depth, convert_linear ? GL_SRGB8_ALPHA8 : GL_SRGB8_ALPHA8);
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

void Framebuffer::Clear(uint width, uint height)
{
    Bind();

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::Bind() 
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::Unbind() 
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::AttachColor(Texture2D* texture, uint attachment, uint mip_level) 
{
    Bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachment, texture->Target(), texture->Id(), mip_level);
    attachments[attach_count++] = GL_COLOR_ATTACHMENT0+attachment;

    glDrawBuffers(attach_count, attachments);
    glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);

    Unbind();
}

void Framebuffer::AttachDepthStencil(Texture2D* texture, uint attachment) 
{
    Bind();

	glFramebufferTexture(GL_FRAMEBUFFER, attachment, texture->Id(), 0);
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

Buffer::Buffer(uint tp, uint usage, size_t size, void* data)
{
    type = tp;

    glGenBuffers(1, &id);

    glBindBuffer(type, id);
    glBufferData(type, size, data, usage);
    glBindBuffer(type, 0);
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &id);
}

void Buffer::Bind()
{
    glBindBuffer(type, id);
}

void Buffer::Unbind()
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

void Buffer::SetData(uint offset, uint size, void* data)
{
    glBindBuffer(type, id);
    glBufferSubData(type, offset, size, data);
    glBindBuffer(type, 0);
}

Buffer* Buffer::CreateVBO(uint usage, uint size, void* data)
{
    return new Buffer(GL_ARRAY_BUFFER, usage, size, data);
}

Buffer* Buffer::CreateIBO(uint usage, uint size, void* data)
{
    return new Buffer(GL_ELEMENT_ARRAY_BUFFER, usage, size, data);
}

VertexArray::VertexArray(Buffer* vbo, Buffer* ibo, VertexAttrib attribs[], uint count)
{
    glGenVertexArrays(1, &id);
    glBindVertexArray(id);

    vbo->Bind();

    if (ibo)
    {
        ibo->Bind();
    }

    for (uint32_t i = 0; i < count; i++)
    {
        glEnableVertexAttribArray(attribs[i].index);

        glVertexAttribPointer(attribs[i].index, attribs[i].num_elements, attribs[i].type, attribs[i].normalize, attribs[i].stride, (void*)(attribs[i].offset));
    }

    glBindVertexArray(0);

    vbo->Unbind();

    if (ibo)
    {
        ibo->Unbind();
    }
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

Program::Program(const Shader* vs, const Shader* fs, unsigned count, const char* log_name)
{
    const Shader* shader_list[] = { vs, fs };
    Init(shader_list, 2, log_name);
}

Program::Program(const Shader** shaders, unsigned count, const char* log_name)
{
    Init(shaders, count, log_name);
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

void Program::BindTexture(uint location, uint unit, const Texture2D* texture)
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(GL_TEXTURE_2D, texture->Id());
    glUniform1i(location, unit);        
}

void Program::BindUniform(uint location, int value)
{
    glUniform1i(location, value);        
}

void Program::BindUniform(uint location, float value)
{
    glUniform1f(location, value);        
}

void Program::BindUniform(uint location, const float2& value)
{
    glUniform2f(location, value.x, value.y);
}

void Program::BindUniform(uint location, const float3& value)
{
    glUniform3f(location, value.x, value.y, value.z);
}

void Program::BindUniform(uint location, const float4& value)
{
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Program::BindUniform(uint location, const float2x2& value)
{
    glUniformMatrix2fv(location, 1, GL_TRUE, reinterpret_cast<const float*>(&value));
}

void Program::BindUniform(uint location, const float3x3& value)
{
    glUniformMatrix3fv(location, 1, GL_TRUE, reinterpret_cast<const float*>(&value));
}

void Program::BindUniform(uint location, const float4x4& value)
{
    glUniformMatrix4fv(location, 1, GL_TRUE, reinterpret_cast<const float*>(&value));
}

void Program::BindUniform(uint location, int count, int* value)
{
    glUniform1iv(location, count, value);
}

void Program::BindUniform(uint location, int count, float* value)
{
    glUniform1fv(location, count, value);
}

void Program::BindUniform(uint location, int count, const float2* value)
{
    glUniform2fv(location, count, reinterpret_cast<const float*>(value));
}

void Program::BindUniform(uint location, int count, const float3* value)
{
    glUniform3fv(location, count, reinterpret_cast<const float*>(value));
}

void Program::BindUniform(uint location, int count, const float4* value)
{
    glUniform4fv(location, count, reinterpret_cast<const float*>(value));
}

void Program::BindUniform(uint location, int count, const float2x2* value)
{
    glUniformMatrix2fv(location, count, GL_TRUE, reinterpret_cast<const float*>(&value));
}

void Program::BindUniform(uint location, int count, const float3x3* value)
{
    glUniformMatrix3fv(location, count, GL_TRUE, reinterpret_cast<const float*>(&value));
}

void Program::BindUniform(uint location, int count, const float4x4* value)
{
    glUniformMatrix4fv(location, count, GL_TRUE, reinterpret_cast<const float*>(&value));
}

void Program::BindSSBO(unsigned binding, const Buffer* buffer)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, buffer->Id());
}

