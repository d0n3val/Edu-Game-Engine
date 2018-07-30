#ifndef _PROGRAMS_H_
#define _PROGRAMS_H_

#include "Module.h"
#include "HashString.h"
#include <map>

class ModulePrograms : public Module
{
public:

    typedef std::pair<char*, unsigned> UniformBinding;

private:

	typedef std::pair<HashString, unsigned> KeyProg;
	typedef std::pair<HashString, const char*> KeyProgDef;

	struct LessKeyProg
	{
		bool operator()(const KeyProg& left, const KeyProg& right) const
		{
			return left.second < right.second || (left.second == right.second && left.first < right.first );
		}
	};

    struct ProgramDef
    {
        char*           vertex        = nullptr;
        char*           fragment      = nullptr;
        char**          macros        = nullptr;
        unsigned        num_macros    = 0;
        UniformBinding* bindings      = nullptr;
        unsigned        num_bindings  = 0;
        mutable char**  data          = nullptr;
    };

	typedef std::map<KeyProg, unsigned, LessKeyProg> ProgramList;
    typedef std::map<HashString, ProgramDef> DefList;

	ProgramList programs;
    DefList     defs;

public:

	explicit ModulePrograms(bool start_active);
	~ModulePrograms();

public:

	void Load              (const char* name, const char* vertex_shader, const char* fragment_shader, 
                            const char** macros, unsigned num_macros, const UniformBinding* uniforms, 
                            unsigned num_uniforms);

    void GenerateVariation (const char* name, unsigned variations);

	void Clear             ();

	int  GetUniformLocation(const char* uniform);
	int  GetAttribLocation (const char* attribute);
	void UseProgram        (const char* name, unsigned variation);
	void UnuseProgram      ();

private:

	void  Compile           (char** data, char* shader_data, unsigned id, unsigned variations, char** macros, unsigned num_macros) const;
    char* LoadFile          (const char* file_name);
};

#endif
