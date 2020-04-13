#ifndef __PERLINPROPERTIES_H__
#define __PERLINPROPERTIES_H__

#include "OGL.h"
#include "Timer.h"

#include <memory>

struct FractalNoiseCfg;

class PerlinProperties
{
public:

    PerlinProperties();
    ~PerlinProperties() = default;

    void Draw(FractalNoiseCfg& info);

private:

    void GenerateTexture(FractalNoiseCfg& info);

private:

    std::unique_ptr<Framebuffer> perlin_fb;
    std::unique_ptr<Texture2D>   perlin_text;
    std::unique_ptr<Texture2D>   perlin_text2;
    std::unique_ptr<Program>     perlin_prog;
    float                        frame = 0.0f;
    Timer                        timer;

};

#endif /* __PERLINPROPERTIES_H__ */


