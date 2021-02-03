#ifndef _PROGRAMS_H_
#define _PROGRAMS_H_

#include "Module.h"
#include "HashString.h"
#include <map>

class ModulePrograms : public Module
{
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
        unsigned        num_bindings  = 0;
        mutable char**  data          = nullptr;
        bool            add_version   = true;
    };

	typedef std::map<KeyProg, unsigned, LessKeyProg> ProgramList;
    typedef std::map<HashString, ProgramDef> DefList;

	ProgramList programs;
    DefList     defs;

public:

	explicit ModulePrograms(bool start_active);
	~ModulePrograms();

public:

	void Load                           (const char* name, const char* vertex_shader, const char* fragment_shader, 
                                        const char** macros, unsigned num_macros, bool version = true);

    void GenerateVariation              (const char* name, unsigned variations);

	void Clear                          ();

    void BindUniformBlock               (const char* program, unsigned variations, const char* block_name, uint block_index);

	int  GetUniformLocation             (const char* uniform);
    int  GetSubroutineUniformLocation   (bool vertex_shader, const char* name);
    int  GetSubroutineIndex             (bool vertex_shader, const char* name);
    void UseProgram                     (const char* name, unsigned variation = 0);
	void UnuseProgram                   ();

private:

	void  Compile           (char** data, char* shader_data, unsigned id, unsigned variations, char** macros, unsigned num_macros, bool version) const;
    char* LoadFile          (const char* file_name);
};

#endif
