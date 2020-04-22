layout(location=0) uniform sampler2D diffuse;
layout(location=1) uniform float     alpha_test;

in vec2 uv0;

void main()
{
    float alpha = texture(diffuse, uv0).a;

    if(alpha < alpha_test)
    {
        discard;
    }
}
