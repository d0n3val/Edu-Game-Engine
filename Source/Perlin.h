#ifndef __PERLIN_H__
#define __PERLIN_H__

struct FractalPerlin3D
{
    float strength     = 10;   // third axys frequency
    float frequency    = 10;   // perlin noise frequency
    uint octaves       = 1;    // number of octaves
    float octave_mult  = 1.5f; // amplitude multiplier
    float octave_scale = 0.5f; // frequency scale
};



#endif /* __PERLIN_H__ */
