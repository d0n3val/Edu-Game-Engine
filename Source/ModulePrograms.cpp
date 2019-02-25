#include "Globals.h"
#include "ModulePrograms.h"

#include "OpenGL.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "mmgr/mmgr.h"

ModulePrograms::ModulePrograms(bool start_active) : Module("Program", start_active)
{
}

ModulePrograms::~ModulePrograms()
{
	Clear();
}

void ModulePrograms::Load(const char* name, const char* vertex_shader, const char* fragment_shader, 
						  const char** macros, unsigned num_macros, const UniformBinding* uniforms,
						  unsigned num_uniforms)
{
    HashString hash(name);

    assert(defs.find(hash) == defs.end());

	ProgramDef& def = defs[hash];
    def.vertex      = LoadFile(vertex_shader);
    def.fragment    = LoadFile(fragment_shader);

    def.macros      = (char**)malloc(sizeof(char*)*num_macros);

    for(unsigned i=0; i < num_macros; ++i)
    {
        def.macros[i] = _strdup(macros[i]);
    }

    def.num_macros   = num_macros;
    def.bindings     = (UniformBinding*)malloc(sizeof(UniformBinding)*num_uniforms);

    for(unsigned i=0; i< num_uniforms; ++i)
    {
        def.bindings[i].first = _strdup(uniforms[i].first);
		def.bindings[i].second = uniforms[i].second;
    }

    def.num_bindings = num_uniforms;
    def.data = (char**)malloc(sizeof(char*)*(def.num_macros + 2));

}

void ModulePrograms::Clear()
{
    for(DefList::iterator it = defs.begin(); it != defs.end(); ++it)
    {
        for(unsigned i=0; i< it->second.num_macros; ++i)
        {
            //free(it->second.macros[i]);
        }

        free(it->second.macros);

        for(unsigned i=0; i< it->second.num_bindings; ++i)
        {
            free(it->second.bindings[i].first);
        }

        free(it->second.bindings);
        free(it->second.data);
        free(it->second.vertex);
        free(it->second.fragment);
    }

	for(ProgramList::iterator it = programs.begin(); it != programs.end(); ++it)
	{
		glDeleteProgram(it->second);
	}

	programs.clear();
}

void ModulePrograms::UseProgram(const char* name, unsigned variations)
{
	KeyProg key;
	key.first = HashString(name);
	key.second = variations;

	ProgramList::iterator it = programs.find(key);

	if(it == programs.end())
    {
        GenerateVariation(name, variations);
		it = programs.find(key);
    }

    if(it != programs.end())
	{
		glUseProgram(it->second);
	}
}

void ModulePrograms::UnuseProgram()
{
		glUseProgram(0);
}

void ModulePrograms::GenerateVariation(const char* name, unsigned variations)
{
	HashString hash(name);
    DefList::iterator it = defs.find(hash);
    if(it != defs.end())
    {
        const ProgramDef& def = it->second;

		KeyProg key(hash, variations);
        unsigned& program_id  = programs[key];
        unsigned vertex_id    = glCreateShader(GL_VERTEX_SHADER);
        unsigned fragment_id  = glCreateShader(GL_FRAGMENT_SHADER);

        Compile(def.data, def.vertex, vertex_id, variations, def.macros, def.num_macros);
        Compile(def.data, def.fragment, fragment_id, variations, def.macros, def.num_macros);

        program_id = glCreateProgram();

        glAttachShader(program_id, vertex_id);
        glAttachShader(program_id, fragment_id);

        glLinkProgram(program_id);

        int len = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &len);
        if(len > 0)
        {
            int written = 0;
            char* info  = (char*)malloc(len);

            glGetProgramInfoLog(program_id, len, &written, info);

            LOG("Program Log Info: %s", info);

            free(info);
        }

        for(unsigned i=0; i< def.num_bindings; ++i)
        {
            unsigned index = glGetUniformBlockIndex(program_id, def.bindings[i].first);
            glUniformBlockBinding(program_id, index, def.bindings[i].second);
        }

        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);
    }
}

void ModulePrograms::Compile(char** data,  char* shader_data, unsigned id, unsigned variations, char** macros, unsigned num_macros) const
{
	data[0] = "#version 430\n";

	for(unsigned i=0; i< num_macros; ++i)
	{
		if((variations & (1 << i)) != 0)
		{
			data[i+1] = (char*)macros[i];
		}
		else
		{
			data[i+1] = "";
		}
	}

    data[num_macros+1] = shader_data;

    glShaderSource(id, num_macros+2, data, 0);
    glCompileShader(id);

    int res = GL_FALSE;
    glGetShaderiv(id, GL_COMPILE_STATUS, &res);

    if(res == GL_FALSE)
    {
        int len = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
        if(len > 0)
        {
            int written = 0;
            char* info  = (char*)malloc(len);

            glGetShaderInfoLog(id, len, &written, info);

            LOG("Log Info: %s", info);

            free(info);
        }
    }
}

char* ModulePrograms::LoadFile(const char* file_name)
{
    char* data = nullptr;

	FILE* file = 0;
	fopen_s(&file, file_name, "rb");

	if(file)
	{
		fseek(file, 0, SEEK_END);
		int size = ftell(file);
		rewind(file);
		data = (char*)malloc(size + 1);

		fread(data, 1, size, file);
		data[size] = 0;

		fclose(file);
	}

    return data;
}

int  ModulePrograms::GetUniformLocation(const char* uniform)
{
	int program = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &program);
	
	if(program != 0)
	{
		return glGetUniformLocation(program, uniform);
	}

	return -1;
}

int ModulePrograms::GetSubroutineUniformLocation(bool vertex_shader, const char* name)
{
	int program = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &program);
	
	if(program != 0)
	{
		return glGetSubroutineUniformLocation(program, vertex_shader ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER, name);
	}

	return -1;
}

int ModulePrograms::GetSubroutineIndex(bool vertex_shader, const char* name)
{
	int program = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &program);
	
	if(program != 0)
	{
		return glGetSubroutineIndex(program, vertex_shader ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER, name);
	}

	return -1;
}
