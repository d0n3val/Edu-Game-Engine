#version 450

in vec2 uv;
out vec4 color;

layout(location = 0) uniform float strength;
layout(location = 1) uniform float frequency;
layout(location = 2) uniform int octaves;
layout(location = 3) uniform float freq_mult;
layout(location = 4) uniform float ampl_scale;
layout(location = 5) uniform float frame;

vec3 random( vec3 p ) 
{
	p = vec3( dot(p,vec3(127.1,311.7, 74.7)),
			  dot(p,vec3(269.5,183.3,246.1)),
			  dot(p,vec3(113.5,271.9,124.6)));

	return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

float noise( in vec3 x )
{
    // grid
    vec3 i = floor(x);
    vec3 w = fract(x);
    
    // quintic interpolant
    vec3 u = w*w*w*(w*(w*6.0-15.0)+10.0);
    vec3 du = 30.0*w*w*(w*(w-2.0)+1.0);
    
    // gradients
    vec3 ga = random( i+vec3(0.0,0.0,0.0) );
    vec3 gb = random( i+vec3(1.0,0.0,0.0) );
    vec3 gc = random( i+vec3(0.0,1.0,0.0) );
    vec3 gd = random( i+vec3(1.0,1.0,0.0) );
    vec3 ge = random( i+vec3(0.0,0.0,1.0) );
	vec3 gf = random( i+vec3(1.0,0.0,1.0) );
    vec3 gg = random( i+vec3(0.0,1.0,1.0) );
    vec3 gh = random( i+vec3(1.0,1.0,1.0) );
    
    // projections
    float va = dot( ga, w-vec3(0.0,0.0,0.0) );
    float vb = dot( gb, w-vec3(1.0,0.0,0.0) );
    float vc = dot( gc, w-vec3(0.0,1.0,0.0) );
    float vd = dot( gd, w-vec3(1.0,1.0,0.0) );
    float ve = dot( ge, w-vec3(0.0,0.0,1.0) );
    float vf = dot( gf, w-vec3(1.0,0.0,1.0) );
    float vg = dot( gg, w-vec3(0.0,1.0,1.0) );
    float vh = dot( gh, w-vec3(1.0,1.0,1.0) );
	
    // interpolations
    return va + u.x*(vb-va) + u.y*(vc-va) + u.z*(ve-va) + u.x*u.y*(va-vb-vc+vd) + u.y*u.z*(va-vc-ve+vg) + u.z*u.x*(va-vb-ve+vf) + (-va+vb+vc-vd+ve-vf-vg+vh)*u.x*u.y*u.z;
}

float fbm( in vec3 x)
{
    float f = 0.0;
    float amp = 1.0;
    float freq = 1.0;

    for(uint i=0; i< octaves; ++i)
    {
        f += amp*noise(x*freq);
        amp *= ampl_scale;
        freq  *= freq_mult;
    }

    return f;
}

void main()
{
    vec3 v;
    v.xy = uv.xy*frequency;
    v.z  = frame;

    float f = fbm(v);

    color = vec4(f, f, f, 1.0);
}
