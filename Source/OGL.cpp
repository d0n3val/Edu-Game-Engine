#include "Globals.h"
#include "OGL.h"
#include "OpenGL.h"

Texture2D::Texture2D() : target(GL_TEXTURE_2D)
{
	glGenTextures(1, &texture);
}

Texture2D::Texture2D(uint t) : target(t)
{
    glGenTextures(1, &texture);
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &texture);
}

void Texture2D::bind(uint unit)
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(target, texture);
}

void Texture2D::unbind(uint unit)
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(target, 0);
}

void Texture2D::initialize(uint width, uint height, uint internal_format, uint format, uint type, void* data, bool mipmaps)
{
    glBindTexture(target, texture);
    glTexImage2D(target, 0, internal_format, width, height, 0, format, type, data);

    if(mipmaps)
    {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 1000);
        glGenerateMipmap(target);
    }
    else
    {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
    }

    glBindTexture(target, 0);
}

void Texture2D::set_data(uint width, uint height, uint mip_level, uint internal_format, uint format, uint type, void* data)
{
    glBindTexture(target, texture);
    glTexImage2D(target, mip_level, internal_format, width, height, 0, format, type, data);
    glBindTexture(target, 0);
}

void Texture2D::set_wrapping(uint wrap_s, uint wrap_t, uint wrap_r)
{
    glBindTexture(target, texture);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_t);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, wrap_r);
    glBindTexture(target, 0);
}

void Texture2D::set_min_max_filer(uint min_filter, uint max_filter)
{
    glBindTexture(target, texture);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, max_filter);
    glBindTexture(target, 0);
}

void Texture2D::generate_mipmaps(uint base, uint max)
{
    glBindTexture(target, texture);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, base);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, max);
	glGenerateMipmap(target);
    glBindTexture(target, 0);
}

Framebuffer::Framebuffer()
{
    glGenFramebuffers(1, &fbo);
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &fbo);
}

void Framebuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::attach_color_texture2D(uint texture, uint attachment, bool msaa, uint mip_level, bool draw, bool read)
{
    bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachment, msaa ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, texture, mip_level);

    glDrawBuffer(draw ? GL_COLOR_ATTACHMENT0+attachment : GL_NONE);
    glReadBuffer(read ? GL_COLOR_ATTACHMENT0+attachment : GL_NONE);

    unbind();
}

void Framebuffer::attach_depth_stencil_texture(uint texture)
{
    bind();
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
    unbind();
}

