#include "Globals.h"

#include "Postprocess.h"
#include "Application.h"
#include "modulehints.h"
#include "ModulePrograms.h"

#include "PostProcessShaderLocations.h"
#include "OpenGL.h"
#include "OGL.h"

#include "Leaks.h"

Postprocess::Postprocess()
{
}

Postprocess::~Postprocess()
{
    if(post_vao != 0)
    {
        glDeleteVertexArrays(1, &post_vao);
    }

    if(post_vbo != 0)
    {
        glDeleteBuffers(1, &post_vbo);
    }

    if(post_vao != 0)
    {
        glDeleteVertexArrays(1, &post_vao);
    }

    if(bloom_tex)
    {
        glDeleteTextures(1, &bloom_tex);
    }

    if(color_tex)
    {
        glDeleteTextures(1, &color_tex);
    }

    if(bloom_blur_tex_0)
    {
        glDeleteTextures(1, &bloom_blur_tex_0);
    }

    if(bloom_blur_tex_1)
    {
        glDeleteTextures(1, &bloom_blur_tex_1);
    }

    if(bloom_fbo)
    {
        glDeleteFramebuffers(1, &bloom_fbo);
    }

    if(bloom_blur_fbo_0)
    {
        glDeleteFramebuffers(1, &bloom_blur_fbo_0);
    }

    if(bloom_blur_fbo_1)
    {
        glDeleteFramebuffers(1, &bloom_blur_fbo_1);
    }
}

void Postprocess::Init()
{
	float vertex_buffer_data[] =
	{
        // positions
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,

        // uvs
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};

    glGenBuffers(1, &post_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, post_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &post_vao);
    glBindVertexArray(post_vao);

    glBindBuffer(GL_ARRAY_BUFFER, post_vbo);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(float)*3*6));

    glBindVertexArray(0);

    glGenFramebuffers(1, &bloom_fbo);
    glGenFramebuffers(1, &bloom_blur_fbo_0);
    glGenFramebuffers(1, &bloom_blur_fbo_1);
}

void Postprocess::Execute(Texture2D* screen, Framebuffer* fbo, unsigned width, unsigned height)
{
    bool msaa  = App->hints->GetBoolValue(ModuleHints::ENABLE_MSAA);

    GenerateBloomFBO(width, height);

    // bloom
    glBindVertexArray(post_vao);
    glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo);
    {
        glClear(GL_COLOR_BUFFER_BIT);
        App->programs->UseProgram("bloom", msaa ? 1 : 0);
        glActiveTexture(GL_TEXTURE0);
        screen->Bind(0, App->programs->GetUniformLocation("image"));
        //glBindTexture(msaa ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, screen_texture);
        //glUniform1i(App->programs->GetUniformLocation("image"), 0); 
        glDrawArrays(GL_TRIANGLES, 0, 6); 
    }

    float weights[] = { 0.198596f,	0.175713f,	0.121703f,	0.065984f,	0.028002f,	0.0093f };

    // blur
    glBindVertexArray(post_vao);
    glBindFramebuffer(GL_FRAMEBUFFER, bloom_blur_fbo_0);
    {
        glClear(GL_COLOR_BUFFER_BIT);
        App->programs->UseProgram("gaussian", 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bloom_tex);
        glUniform1i(App->programs->GetUniformLocation("image"), 0); 
        glUniform1fv(App->programs->GetUniformLocation("weight"), sizeof(weights)/sizeof(float), weights);
        glDrawArrays(GL_TRIANGLES, 0, 6); 
    }

    glBindVertexArray(post_vao);
    glBindFramebuffer(GL_FRAMEBUFFER, bloom_blur_fbo_1);
    {
        glClear(GL_COLOR_BUFFER_BIT);
        App->programs->UseProgram("gaussian", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bloom_blur_tex_0);
        glUniform1i(App->programs->GetUniformLocation("image"), 0); 
        glUniform1fv(App->programs->GetUniformLocation("weight"), sizeof(weights)/sizeof(float), weights);
        glDrawArrays(GL_TRIANGLES, 0, 6); 
    }

    fbo->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int flags = App->hints->GetBoolValue(ModuleHints::ENABLE_BLOOM) ? 1 : 0;
    flags = flags | (App->hints->GetBoolValue(ModuleHints::ENABLE_GAMMA) ? 1 << 1 : 0);

    App->programs->UseProgram("postprocess", flags);

    unsigned indices[NUM_POSPROCESS_SUBROUTINES];

    indices[TONEMAP_LOCATION] = App->hints->GetIntValue(ModuleHints::TONEMAPPING);

    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, sizeof(indices)/sizeof(unsigned), indices);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glUniform1i(SCREEN_TEXTURE_LOCATION, 0); 

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloom_blur_tex_1);
    glUniform1i(BLOOM_TEXTURE_LOCATION, 1); 

    glDrawArrays(GL_TRIANGLES, 0, 6); 

    App->programs->UnuseProgram();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Postprocess::GenerateBloomFBO(unsigned width, unsigned height)
{
    if(width != bloom_width || height != bloom_height)
    {
        if(bloom_blur_tex_0 != 0)
        {
            glDeleteTextures(1, &bloom_blur_tex_0);
        }

        if(bloom_blur_tex_1 != 0)
        {
            glDeleteTextures(1, &bloom_blur_tex_1);
        }

        if(width != 0 && height != 0)
        {
            // split bloom color
            glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo);
            glGenTextures(1, &color_tex);
            glBindTexture(GL_TEXTURE_2D, color_tex);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_tex, 0);

            glGenTextures(1, &bloom_tex);
            glBindTexture(GL_TEXTURE_2D, bloom_tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bloom_tex, 0);

            unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
            glDrawBuffers(2, attachments);  

            // blur 

            glBindFramebuffer(GL_FRAMEBUFFER, bloom_blur_fbo_0);
            glGenTextures(1, &bloom_blur_tex_0);
            glBindTexture(GL_TEXTURE_2D, bloom_blur_tex_0);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom_blur_tex_0, 0);

            glDrawBuffer(GL_COLOR_ATTACHMENT0);

            glBindFramebuffer(GL_FRAMEBUFFER, bloom_blur_fbo_1);
            glGenTextures(1, &bloom_blur_tex_1);
            glBindTexture(GL_TEXTURE_2D, bloom_blur_tex_1);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom_blur_tex_1, 0);

            glDrawBuffer(GL_COLOR_ATTACHMENT0);
        }

		bloom_width  = width;
		bloom_height = height;
    }
}
