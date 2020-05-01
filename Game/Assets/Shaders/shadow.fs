layout(location=1) uniform float     alpha_test;
layout(location=2) uniform sampler2D diffuse;

in vec2 uv0;

void main()
{
    float alpha = texture(diffuse, uv0).a;

    if(alpha < alpha_test)
    {
        discard;
    }
}
