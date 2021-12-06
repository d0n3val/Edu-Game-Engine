layout(location=0) out vec4 color;
layout(location=1) out vec4 bloom;

in vec2 uv;

#if MSAA 

uniform sampler2DMS image;
uniform sampler2DMS depth;

vec3 GetTexel(in vec2 uv, sampler2DMS img)
{
    ivec2 vp = textureSize(image);
    vp = ivec2(vec2(vp)*uv);

    vec4 sample1 = texelFetch(img, vp, 0);
    vec4 sample2 = texelFetch(img, vp, 1);
	vec4 sample3 = texelFetch(img, vp, 2);
	vec4 sample4 = texelFetch(img, vp, 3);

    return (sample1.rgb + sample2.rgb + sample3.rgb + sample4.rgb) / 4.0f;
}

#else

uniform sampler2D image;
uniform sampler2D depth;

vec3 GetTexel(in vec2 uv, sampler2D img)
{
    return texture(img, uv).rgb;
}

#endif 

void main()
{
    color = vec4(GetTexel(uv, image), 1.0);
    float depthValue = GetTexel(uv, depth).r;

    float bright = dot(color.rgb, vec3(0.3, 0.6, 0.2));

    if(depthValue < 1.0 && bright > 1.0) 
        bloom = color;
    else 
        bloom = vec4(0.0, 0.0, 0.0, 1.0);
}

