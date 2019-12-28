#include "Globals.h"
#include "OGL.h"
#include "OpenGL.h"

Texture2D::Texture2D(uint target, uint samples, uint width, uint height, uint internal_format, bool fixed_samples) : tex_target(target)
{
	glGenTextures(1, &texture);
    glBindTexture(tex_target, texture);

    glTexImage2DMultisample(tex_target, samples, internal_format, width, height, fixed_samples ? GL_TRUE : GL_FALSE);

    default_initialize(false);

    glBindTexture(tex_target, 0);
}

Texture2D::Texture2D(uint target, uint width, uint height, uint internal_format, uint format, uint type, void* data, bool mipmaps) : tex_target(target)
{
	glGenTextures(1, &texture);
    glBindTexture(tex_target, texture);

    glTexImage2D(tex_target, 0, internal_format, width, height, 0, format, type, data);
    default_initialize(mipmaps);

    glBindTexture(tex_target, 0);
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &texture);
}

void Texture2D::bind(uint unit) 
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(tex_target, texture);
}

void Texture2D::unbind(uint unit) 
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(tex_target, 0);
}

void Texture2D::default_initialize(bool mipmaps)
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

void Texture2D::set_data(uint width, uint height, uint mip_level, uint internal_format, uint format, uint type, void* data)
{
    glBindTexture(tex_target, texture);
    glTexImage2D(tex_target, mip_level, internal_format, width, height, 0, format, type, data);
    glBindTexture(tex_target, 0);
}

void Texture2D::set_wrapping(uint wrap_s, uint wrap_t, uint wrap_r)
{
    glBindTexture(tex_target, texture);
    glTexParameteri(tex_target, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(tex_target, GL_TEXTURE_WRAP_T, wrap_t);
    glTexParameteri(tex_target, GL_TEXTURE_WRAP_R, wrap_r);
    glBindTexture(tex_target, 0);
}

void Texture2D::set_min_max_filer(uint min_filter, uint max_filter)
{
    glBindTexture(tex_target, texture);
    glTexParameteri(tex_target, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(tex_target, GL_TEXTURE_MAG_FILTER, max_filter);
    glBindTexture(tex_target, 0);
}

void Texture2D::generate_mipmaps(uint base, uint max)
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

void Framebuffer::attach_color(Texture2D* texture, uint attachment, uint mip_level, bool draw, bool read) 
{
    bind();

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachment, texture->target(), texture->id(), mip_level);

    glDrawBuffer(draw ? GL_COLOR_ATTACHMENT0+attachment : GL_NONE);
    glReadBuffer(read ? GL_COLOR_ATTACHMENT0+attachment : GL_NONE);

    unbind();
}

void Framebuffer::attach_depth_stencil(Texture2D* texture) 
{
    bind();
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture->id(), 0);
    unbind();
}

void Framebuffer::blit_to(Framebuffer* target, uint src_x0, uint src_y0, uint src_x1, uint src_y1, uint dst_x0, uint dst_y0, 
                          uint dst_x1, uint dst_y1, uint flags, uint filter) 
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target->id());
    glBlitFramebuffer(src_x0, src_y0, src_x1, src_y1, dst_x0, dst_y0, dst_x1, dst_y1, flags, filter);
}
