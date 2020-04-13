#include "Globals.h"

#include "Noise.h"

#include "Math.h"

float Mix(const float& a, const float& b, float lambda)
{
    return a * (1.0f - lambda) + b * lambda;
}

float3 Sin(const float3& a)
{
    return float3(Sin(a.x), Sin(a.y), Sin(a.z));
}

float3 Floor(const float3& a)
{
    return float3(Floor(a.x), Floor(a.y), Floor(a.z));
}

float3 Fract(const float3& a)
{
    return float3(a.x-Floor(a.x), a.y-Floor(a.y), a.z-Floor(a.z));
}

float Fract(const float& a)
{
    return a-floor(a);
}

float3 RandomGradient( const float3& p )
{
	float3 r = float3( Dot(p, float3(127.1f,311.7f, 74.7f)),
			           Dot(p, float3(269.5f,183.3f,246.1f)),
			           Dot(p, float3(113.5f,271.9f,124.6f)));

	return Fract(Sin(r)*43758.5453123f)*2.0f-float3(1.0f);
}

float RandomValue( const float3& p ) 
{
    float3 p2  = Fract( p*0.3183099f + float3(0.71f,0.113f,0.419f))*50.0f;
    return Fract( p2.x*p2.y*p2.z*(p2.x+p2.y+p2.z));
}

float ValueNoise(const float3& x)
{
    float3 i = Floor(x);
    float3 f = Fract(x);

    // quintic interpolation
    float3 u = f* f* f* (f * (f * 6.0 - float3(15.0)) + float3(10.0));
    
    return Mix(Mix(Mix( RandomValue(i+float3(0,0,0)), 
                        RandomValue(i+float3(1,0,0)),u.x),
                   Mix( RandomValue(i+float3(0,1,0)), 
                        RandomValue(i+float3(1,1,0)),u.x),u.y),
               Mix(Mix( RandomValue(i+float3(0,0,1)), 
                        RandomValue(i+float3(1,0,1)),u.x),
                   Mix( RandomValue(i+float3(0,1,1)), 
                        RandomValue(i+float3(1,1,1)),u.x),u.y),u.z);
}

float GradientNoise( const float3& x ) 
{
    float3 i = Floor(x);
    float3 f = Fract(x);
	
    // quintic interpolation
    float3 u = f*f*f*(f*(f*6.0-float3(15.0))+float3(10.0));

    return Mix( Mix( Mix( Dot( RandomGradient( i + float3(0.0,0.0,0.0) ), f - float3(0.0,0.0,0.0) ), 
                          Dot( RandomGradient( i + float3(1.0,0.0,0.0) ), f - float3(1.0,0.0,0.0) ), u.x),
                     Mix( Dot( RandomGradient( i + float3(0.0,1.0,0.0) ), f - float3(0.0,1.0,0.0) ), 
                          Dot( RandomGradient( i + float3(1.0,1.0,0.0) ), f - float3(1.0,1.0,0.0) ), u.x), u.y),
                Mix( Mix( Dot( RandomGradient( i + float3(0.0,0.0,1.0) ), f - float3(0.0,0.0,1.0) ), 
                          Dot( RandomGradient( i + float3(1.0,0.0,1.0) ), f - float3(1.0,0.0,1.0) ), u.x),
                     Mix( Dot( RandomGradient( i + float3(0.0,1.0,1.0) ), f - float3(0.0,1.0,1.0) ), 
                          Dot( RandomGradient( i + float3(1.0,1.0,1.0) ), f - float3(1.0,1.0,1.0) ), u.x), u.y), u.z );
}

float FractalNoise( const FractalNoiseCfg& cfg, const float3& x )
{
    float f    = 0.0;
    float amp  = 1.0;
    float freq = 1.0f;

    float3 value(x.x*cfg.frequency, x.y*cfg.frequency, x.z*cfg.duration); 

    for(uint i=0; i< cfg.octaves; ++i)
    {
        f    += amp* GradientNoise(value*freq);
        amp  *= cfg.octave_scale;
        freq *= cfg.octave_mult;
    }

    return f;
}

void LoadNoiseCfg(FractalNoiseCfg& cfg,const Config& config)
{
    cfg.duration     = config.GetFloat("Duration", 10.0f);
    cfg.strength     = config.GetFloat("Strength", 1.0f);
    cfg.frequency    = config.GetFloat("Frequency", 2.5f);
    cfg.octaves      = config.GetUInt("Octaves", 1);
    cfg.octave_mult  = config.GetFloat("OctaveMult", 1.5f);
    cfg.octave_scale = config.GetFloat("OctaveScale", 0.5f);
}

void SaveNoiseCfg(const FractalNoiseCfg& cfg, Config& config)
{
    config.AddFloat("Duration", cfg.duration);
    config.AddFloat("Strength", cfg.strength);
    config.AddFloat("Frequency", cfg.frequency);
    config.AddUInt("Octaves", cfg.octaves);
    config.AddFloat("OctaveMult", cfg.octave_mult);
    config.AddFloat("OctaveScale", cfg.octave_scale);
}
