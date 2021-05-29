layout(location=0) out vec4 color;
layout(location=1) out vec4 bloom;

in vec2 uv;

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
    color = vec4(GetTexel(uv), 1.0);

    float bright = dot(color.rgb, vec3(0.3, 0.6, 0.2));

    if(bright > 1.0) bloom = color;
    else bloom = vec4(0.0, 0.0, 0.0, 1.0);
}

