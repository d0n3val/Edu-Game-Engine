#version 440

out vec4 color;
in vec2 uv;

uniform sampler2D image;

void main()
{
    vec2 tex_offset = 1.0 / textureSize(image, 0); 
    vec3 result = vec3(0.0);

    for(int i = 0; i < 3; ++i)
    {
        for(int j=0; j<3; ++j)
        {
            result += texture(image, uv + vec2(tex_offset.x * (i-1), tex_offset.y * (j-1))).rgb; 
        }
    }

    color = vec4(result/9.0, 1.0);
}

