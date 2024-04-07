#version 460

layout(binding = 0) uniform sampler2D colorTex;

in vec2 uv;

out vec4 color;

void main()
{
    color = texture(colorTex, uv);
}

