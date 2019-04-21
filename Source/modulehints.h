#ifndef _HINTS_h_
#define _HINTS_h_

#include "Module.h"

class ModuleHints : public Module
{
public:
	enum Hint
	{
		ENABLE_NORMAL_MAPPING = 0,
        SHOW_TANGENT_SPACE,
		ENABLE_SPECULAR_MAPPING,
		ENABLE_SHADOW_MAPPING,
		SHOW_SHADOW_CLIPPING,
		ENABLE_SHADOW_FRONT_CULLING,
		SHADOW_BIAS,
		SHADOW_RESOLUTION,
		UPDATE_SHADOW_VOLUME,
		METRIC_PROPORTION,
		ENABLE_FRESNEL,
        TONEMAPPING,
        ENABLE_MSAA,
        ENABLE_GAMMA,
		COUNT
	};

private:

    enum EType
    {
        TYPE_FLOAT = 0,
        TYPE_BOOL  = 1,
        TYPE_INT
    };

    struct
    {

        union
        {
            int   ivalue;
            float fvalue;
            bool  bvalue;
        }     value;
        EType type;

    } hints[COUNT];

public:

    ModuleHints();
    ~ModuleHints();

    int     GetIntValue     (Hint hint) const;
    void    SetIntValue     (Hint hint, int value);

    bool    GetBoolValue    (Hint hint) const;
    void    SetBoolValue    (Hint hint, bool value);

    float   GetFloatValue   (Hint hint) const;
    void    SetFloatValue   (Hint hint, float value);

	bool    Init            (Config* config) override;
	void    Save            (Config* config) const override;
};

inline bool ModuleHints::GetBoolValue(Hint hint) const
{
    return hints[hint].type == TYPE_BOOL ? hints[hint].value.bvalue : false;
}

inline void ModuleHints::SetBoolValue(Hint hint, bool value) 
{
    if(hints[hint].type == TYPE_BOOL) 
    {
        hints[hint].value.bvalue = value;
    }
}

inline float ModuleHints::GetFloatValue(Hint hint) const
{
    return hints[hint].type == TYPE_FLOAT ? hints[hint].value.fvalue : 0.0f;
}

inline void ModuleHints::SetFloatValue(Hint hint, float value) 
{
    if(hints[hint].type == TYPE_FLOAT) 
    {
        hints[hint].value.fvalue = value;
    }
}

inline int ModuleHints::GetIntValue(Hint hint) const
{
    return hints[hint].type == TYPE_INT ? hints[hint].value.ivalue : 0;
}

inline void ModuleHints::SetIntValue(Hint hint, int value)
{
    if(hints[hint].type == TYPE_INT) 
    {
        hints[hint].value.ivalue = value;
    }
}

#endif /* _HINTS_h_ */
