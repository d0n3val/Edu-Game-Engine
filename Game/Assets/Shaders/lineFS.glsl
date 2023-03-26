#version 460 
#extension GL_ARB_shading_language_include : require

#include "/shaders/common.glsl"

#define STARTING 0
#define PLAYING 1
#define STOPPING 2
#define STOPPED 3

uniform sampler2D colorTex;
uniform float time;
uniform float fadeTime;
uniform int state;

in vec2 uv;
in vec3 incolor;

out vec4 color;

void main()
{
    color = texture(colorTex, uv);
    color.rgb *= incolor;

    if(state == STARTING)
    {
        if(uv.x*fadeTime > time ) color = vec4(0.0);
    }
    else if(state == STOPPING)
    {
        if(uv.x*fadeTime < time) color = vec4(0.0);
    }
}
