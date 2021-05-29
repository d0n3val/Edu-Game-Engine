// \todo: change subroutine x ifdef 
subroutine vec3 ToneMapping(const vec3 hdr);

in vec2 uv;
out vec4 color;

layout(location=0) uniform sampler2D screen_texture;

#if BLOOM
layout(location=1) uniform sampler2D bloom_texture;
#endif

layout(location=0) subroutine uniform ToneMapping tonemap;

vec3 ACESFilm(in vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;

    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

vec3 Uncharted2Tonemap(in vec3 x)
{
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;

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
       
layout(index=2) subroutine(ToneMapping) vec3 aces_tonemap(const vec3 hdr)
{
    return ACESFilm(hdr);
}
       
layout(index=3) subroutine(ToneMapping) vec3 no_tonemap(const vec3 hdr)
{
    return hdr;
}
       
void main()
{
    vec4 hdr = texture(screen_texture, uv);

#if BLOOM
    vec4 bloom = texture(bloom_texture, uv);

    hdr.rgb += bloom.rgb;
#endif

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
