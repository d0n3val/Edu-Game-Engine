out vec4 color;
in vec2 uv;

uniform sampler2D image;

//float weight[3] = float[] (0.38774,	0.24477, 0.06136);
float weight[4] = float[] (0.383103, 0.241843, 0.060626, 0.00598);
//float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec2 tex_offset = 1.0 / textureSize(image, 0); 
    vec3 result = texture(image, uv).rgb * weight[0]; // current fragment's contribution

#if HORIZONTAL
    for(int i = 1; i < 4; ++i)
    {
        result += texture(image, uv + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        result += texture(image, uv - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
    }
#else
    for(int i = 1; i < 4; ++i)
    {
        result += texture(image, uv + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        result += texture(image, uv - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
    }
#endif

    color = vec4(result, 1.0);
}

