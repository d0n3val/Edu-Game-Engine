#version 450

in vec2 uv;
out vec4 color;

layout(location = 0) uniform float duration;
layout(location = 1) uniform float strength;
layout(location = 2) uniform float frequency;
layout(location = 3) uniform int octaves;
layout(location = 4) uniform float freq_mult;
layout(location = 5) uniform float ampl_scale;
layout(location = 6) uniform float frame;
layout(location = 7) uniform sampler2D tex;

vec3 gradient_random( vec3 p ) 
{
	p = vec3( dot(p,vec3(127.1,311.7, 74.7)),
			  dot(p,vec3(269.5,183.3,246.1)),
			  dot(p,vec3(113.5,271.9,124.6)));

	return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}


float value_random(vec3 p)  // replace this by something better
{
    p  = 50.0*fract( p*0.3183099 + vec3(0.71,0.113,0.419));
    return -1.0+2.0*fract( p.x*p.y*p.z*(p.x+p.y+p.z) );
}

float gradient_noise( in vec3 x )
{
    vec3 i = floor( x);
    vec3 f = fract( x );
	
    // quintic interpolation
    vec3 u = f*f*f*(f*(f*6.0-15.0)+10.0);

    return mix( mix( mix( dot( gradient_random( i + vec3(0.0,0.0,0.0) ), f - vec3(0.0,0.0,0.0) ), 
                          dot( gradient_random( i + vec3(1.0,0.0,0.0) ), f - vec3(1.0,0.0,0.0) ), u.x),
                     mix( dot( gradient_random( i + vec3(0.0,1.0,0.0) ), f - vec3(0.0,1.0,0.0) ), 
                          dot( gradient_random( i + vec3(1.0,1.0,0.0) ), f - vec3(1.0,1.0,0.0) ), u.x), u.y),
                mix( mix( dot( gradient_random( i + vec3(0.0,0.0,1.0) ), f - vec3(0.0,0.0,1.0) ), 
                          dot( gradient_random( i + vec3(1.0,0.0,1.0) ), f - vec3(1.0,0.0,1.0) ), u.x),
                     mix( dot( gradient_random( i + vec3(0.0,1.0,1.0) ), f - vec3(0.0,1.0,1.0) ), 
                          dot( gradient_random( i + vec3(1.0,1.0,1.0) ), f - vec3(1.0,1.0,1.0) ), u.x), u.y), u.z );
}

float value_noise( in vec3 x )
{
    vec3 i = floor(x);
    vec3 f = fract(x);

    // quintic interpolation
    vec3 u = f*f*f*(f*(f*6.0-15.0)+10.0);
	
    return mix(mix(mix( value_random(i+vec3(0,0,0)), 
                        value_random(i+vec3(1,0,0)),u.x),
                   mix( value_random(i+vec3(0,1,0)), 
                        value_random(i+vec3(1,1,0)),u.x),u.y),
               mix(mix( value_random(i+vec3(0,0,1)), 
                        value_random(i+vec3(1,0,1)),u.x),
                   mix( value_random(i+vec3(0,1,1)), 
                        value_random(i+vec3(1,1,1)),u.x),u.y),u.z);
}

float fbm( in vec3 x)
{
    float f = 0.0;
    float amp = 1.0;
    float freq = 1.0;

    for(uint i=0; i< octaves; ++i)
    {
        f += amp*gradient_noise(x*freq);
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

    float f = clamp(fbm(v)*0.5+0.5, 0.0, 1.0);

    color = vec4(f, f, f, 1.0);
}
