// \todo: change subroutine x ifdef 
subroutine vec3 ToneMapping(const vec3 hdr);

in vec2 uv;
out vec4 color;

#if MSAA
layout(location=0) uniform sampler2DMS screen_texture;
layout(location=1) uniform sampler2DMS bloom_texture;
#else 
layout(location=0) uniform sampler2D screen_texture;
layout(location=1) uniform sampler2D bloom_texture;
#endif 


layout(location=0) subroutine uniform ToneMapping tonemap;

vec3 Uncharted2Tonemap(vec3 x)
{
    // Filmic tone mapping
    // https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting

    const float A = 0.22; 
    const float B = 0.3;
    const float C = 0.1;
    const float D = 0.2;
    const float E = 0.01;
    const float F = 0.30;

    return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

layout(index=0) subroutine(ToneMapping) vec3 uncharted2_tonemap(const vec3 hdr)
{
    const float W         = 11.2;
    const vec3 whiteScale = 1.0/Uncharted2Tonemap(vec3(W));

    vec3 curr = Uncharted2Tonemap(2.0*hdr);

    return curr*whiteScale;
}

layout(index=1) subroutine(ToneMapping) vec3 reinhard_tonemap(const vec3 hdr)
{
    return hdr / (hdr + vec3(1.0));
}
       
layout(index=2) subroutine(ToneMapping) vec3 no_tonemap(const vec3 hdr)
{
    return hdr;
}
       
void main()
{
#if MSAA 
    ivec2 vp = textureSize(screen_texture);
    vp = ivec2(vec2(vp)*uv);

    vec4 sample1 = texelFetch(screen_texture, vp, 0);
    vec4 sample2 = texelFetch(screen_texture, vp, 1);
	vec4 sample3 = texelFetch(screen_texture, vp, 2);
	vec4 sample4 = texelFetch(screen_texture, vp, 3);

	vec4 hdr = (sample1 + sample2 + sample3 + sample4) / 4.0f;

    sample1 = texelFetch(bloom_texture, vp, 0);
    sample2 = texelFetch(bloom_texture, vp, 1);
	sample3 = texelFetch(bloom_texture, vp, 2);
	sample4 = texelFetch(bloom_texture, vp, 3);

	vec4 bloom = (sample1 + sample2 + sample3 + sample4) / 4.0f;

#else
    vec4 hdr = texture(screen_texture, uv);
    vec4 bloom = texture(bloom_texture, uv);
#endif

    hdr.rgb += bloom.rgb;

    vec3 mapped = tonemap(hdr.rgb);

    // \todo: original filmic tonemapping from 
    // https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting and
    // http://filmicworlds.com/blog/filmic-tonemapping-operators/

    // gamma correction
#if GAMMA
    color = vec4(pow(mapped, vec3(1.0 / 2.2)), 1.0);
#else 
    color = vec4(mapped, 1.0);
#endif
}
