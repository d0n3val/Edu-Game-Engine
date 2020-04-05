#include "Globals.h"
#include "OGL.h"
#include "OpenGL.h"

Texture2D::Texture2D(uint target, uint samples, uint width, uint height, uint internal_format, bool fixed_samples) : tex_target(target)
{
	glGenTextures(1, &texture);
    glBindTexture(tex_target, texture);

    glTexImage2DMultisample(tex_target, samples, internal_format, width, height, fixed_samples ? GL_TRUE : GL_FALSE);

    DefaultInitialize(false);

    glBindTexture(tex_target, 0);
}

Texture2D::Texture2D(uint target, uint width, uint height, uint internal_format, uint format, uint type, void* data, bool mipmaps) : tex_target(target)
{
	glGenTextures(1, &texture);
    glBindTexture(tex_target, texture);

    glTexImage2D(tex_target, 0, internal_format, width, height, 0, format, type, data);
    DefaultInitialize(mipmaps);

    glBindTexture(tex_target, 0);
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &texture);
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

void Texture2D::DefaultInitialize(bool mipmaps)
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

void Texture2D::SetData(uint width, uint height, uint mip_level, uint internal_format, uint format, uint type, void* data)
{
    glBindTexture(tex_target, texture);
    glTexImage2D(tex_target, mip_level, internal_format, width, height, 0, format, type, data);
    glBindTexture(tex_target, 0);
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

