#ifndef _HINTS_h_
#define _HINTS_h_

#include "Module.h"

#include "Math.h"

#include<string>
#include<unordered_map>
#include<variant>

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
		ENABLE_CASCADE_SHADOW,
		SHADOW_BIAS,
        SHADOW_SLOPEBIAS,

		SHADOW_CASCADE_0_DEPTH,
		SHADOW_CASCADE_0_RES,
        SHADOW_CASCADE_0_UPDATE,

		SHADOW_CASCADE_1_DEPTH,
		SHADOW_CASCADE_1_RES,
        SHADOW_CASCADE_1_UPDATE,

		SHADOW_CASCADE_2_DEPTH,
		SHADOW_CASCADE_2_RES,
        SHADOW_CASCADE_2_UPDATE,

        SHADOW_SHOW_CASCADES,

        SHADOW_ENABLE_SOFT,
        SHADOW_PCF_SIZE,

		UPDATE_SHADOW_VOLUME,
        SHOW_SHADOW_MAP,
		METRIC_PROPORTION,
		ENABLE_FRESNEL,
        TONEMAPPING,
        ENABLE_MSAA,
        ENABLE_GAMMA,
        ENABLE_BLOOM,
        SHOW_PARTICLE_BILLBOARDS,
        FOG_TYPE,
        DIST_FOG_COLOUR,
        DIST_FOG_MIN,
        DIST_FOG_MAX,
        DIST_FOG_CURVE,
		COUNT
	};

    typedef std::variant<float, bool, int, float2, float3> DValue;

private:

    enum EType
    {
        TYPE_FLOAT  = 0,
        TYPE_BOOL   = 1,
        TYPE_INT    = 2,
        TYPE_FLOAT2 = 3,
        TYPE_FLOAT3 = 4,
        TYPE_FLOAT4 = 5
    };

    struct
    {

        union
        {
            int    ivalue;
            float  fvalue;
            bool   bvalue;
            float  f2value[2];
            float  f3value[3];
            float  f4value[4];
        }     value;
        EType type;
    } hints[COUNT];


    struct DHint
    {
        std::string name;
        DValue value;
    };

    typedef std::unordered_map<std::string, DValue> DList;

    DList   dhints;

public:

    ModuleHints();
    ~ModuleHints();

    void            SetDHint(const std::string& name, const DValue& value);
    const DValue&   GetDHint(const std::string& name, const DValue& defaultDValue);

    template<class T>
    T               GetDHintValue(const std::string& name, const T& defaultValue) { return std::get<T>(GetDHint(name, DValue(defaultValue))); }


    template<class Callable>
    void            EnumerateDHints(Callable&& function) const; 

    int             GetIntValue     (Hint hint) const;
    void            SetIntValue     (Hint hint, int value);

    bool            GetBoolValue    (Hint hint) const;
    void            SetBoolValue    (Hint hint, bool value);

    float           GetFloatValue   (Hint hint) const;
    void            SetFloatValue   (Hint hint, float value);

    float2          GetFloat2Value  (Hint hint) const;
    void            SetFloat2Value  (Hint hint, const float2& value);

    float3          GetFloat3Value(Hint hint) const;
    void            SetFloat3Value(Hint hint, const float3& value);

    float4          GetFloat4Value(Hint hint) const;
    void            SetFloat4Value(Hint hint, const float4& value);

	bool            Init            (Config* config) override;
	void            Save            (Config* config) const override;
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

inline float2 ModuleHints::GetFloat2Value  (Hint hint) const
{
    return hints[hint].type == TYPE_FLOAT2 ? float2(hints[hint].value.f2value[0], hints[hint].value.f2value[1]): float2(.0f, .0f);
}

inline void ModuleHints::SetFloat2Value  (Hint hint, const float2& value)
{
    if(hints[hint].type == TYPE_FLOAT2) 
    {
        hints[hint].value.f2value[0] = value[0];
        hints[hint].value.f2value[1] = value[1];
    }
}

inline float3 ModuleHints::GetFloat3Value(Hint hint) const
{
    return hints[hint].type == TYPE_FLOAT3 ? float3(hints[hint].value.f3value[0], hints[hint].value.f3value[1], hints[hint].value.f3value[2]) : float3(.0f, .0f, .0f);
}

inline void ModuleHints::SetFloat3Value(Hint hint, const float3& value)
{
    if (hints[hint].type = TYPE_FLOAT3)
    {
        hints[hint].value.f3value[0] = value[0];
        hints[hint].value.f3value[1] = value[1];
        hints[hint].value.f3value[2] = value[2];
    }
}

inline float4 ModuleHints::GetFloat4Value(Hint hint) const
{
    return hints[hint].type == TYPE_FLOAT4 ? float4(hints[hint].value.f4value[0], hints[hint].value.f4value[1], 
                                                    hints[hint].value.f4value[2], hints[hint].value.f4value[3]) : float4(.0f, .0f, .0f, .0f);
}

inline void ModuleHints::SetFloat4Value(Hint hint, const float4 &value)
{
    if (hints[hint].type = TYPE_FLOAT4)
    {
        hints[hint].value.f4value[0] = value[0];
        hints[hint].value.f4value[1] = value[1];
        hints[hint].value.f4value[2] = value[2];
        hints[hint].value.f4value[3] = value[3];
    }
}

template <class Callable>
void ModuleHints::EnumerateDHints(Callable &&function) const
{
    for(auto it = dhints.begin(); it != dhints.end(); ++it ) function(it->first, it->second);
}

#endif /* _HINTS_h_ */
