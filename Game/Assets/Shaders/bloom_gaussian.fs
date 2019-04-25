out vec4 color;
in vec2 uv;


float weight[3] = float[] (0.38774,	0.24477, 0.06136);

#if MSAA 

uniform sampler2DMS image;

vec3 GetTexel(in vec2 uv)
{
    ivec2 vp = textureSize(image);
    vp = ivec2(vec2(vp)*uv);

    vec4 sample1 = texelFetch(image, vp, 0);
    vec4 sample2 = texelFetch(image, vp, 1);
	vec4 sample3 = texelFetch(image, vp, 2);
	vec4 sample4 = texelFetch(image, vp, 3);

    return (sample1.rgb + sample2.rgb + sample3.rgb + sample4.rgb) / 4.0f;
}

#else

uniform sampler2D image;

vec3 GetTexel(in vec2 uv)
{
    return texture(image, uv).rgb;
}

#endif 

void main()
{
#if MSAA 
    vec2 tex_offset = 1.0 / textureSize(image); 
#else
    vec2 tex_offset = 1.0 / textureSize(image, 0); 
#endif
    vec3 result = GetTexel(uv) * weight[0]; // current fragment's contribution

#if HORIZONTAL
    for(int i = 1; i < 3; ++i)
    {
        result += GetTexel(uv + vec2(tex_offset.x * i, 0.0)) * weight[i];
        result += GetTexel(uv - vec2(tex_offset.x * i, 0.0)) * weight[i];

        //result += texture(image, uv + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        //result += texture(image, uv - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
    }
#else
    for(int i = 1; i < 3; ++i)
    {
        result += GetTexel(uv + vec2(0.0, tex_offset.y * i))* weight[i];
        result += GetTexel(uv - vec2(0.0, tex_offset.y * i))* weight[i];

        //result += texture(image, uv + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        //result += texture(image, uv - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
    }
#endif

    color = vec4(result, 1.0);
}

