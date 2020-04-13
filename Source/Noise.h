#ifndef __PERLIN_H__
#define __PERLIN_H__

#include "Config.h"
#include "Math.h"

struct FractalNoiseCfg
{
    float duration     = 10.0f;   // third axys frequency
    float strength     = 0.0f;    // third axys frequency
    float frequency    = 2.5;     // perlin noise frequency
    uint octaves       = 1;       // number of octaves
    float octave_mult  = 1.5f;    // amplitude multiplier
    float octave_scale = 0.5f;    // frequency scale
};


float GradientNoise( const float3& x );
float FractalNoise(const FractalNoiseCfg& cfg, const float3& x );
void  LoadNoiseCfg(FractalNoiseCfg& cfg, const Config& config);
void  SaveNoiseCfg(const FractalNoiseCfg& cfg, Config& config);


#endif /* __PERLIN_H__ */
