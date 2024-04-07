#version 460

#if BLUR
layout(binding = 0) uniform sampler2D blurredTex;
#endif 
layout(binding = 1) uniform sampler2D fogTex;

in vec2 uv;

out vec4 color;

void main()
{
    color = texture(fogTex, uv);
#if BLUR
    color += texture(blurredTex, uv);
#endif 
}

