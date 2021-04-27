#version 440 

#define NUM_SAMPLES 1024
#define PI 3.1415926
#define TWO_PI 2.0*PI
#define HALF_PI 0.5*PI

out vec4 frag_color;

in vec3 coords;

uniform samplerCube skybox;

float radicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley2D(uint i, uint N) 
{
    return vec2(float(i)/float(N), radicalInverse_VdC(i));
}

vec3 hemisphereCosineSample(in vec2 rand)
{
     float phi = rand[0] * 2.0 * PI;
     float r   = sqrt(rand[1]);

     return vec3(r*cos(phi), r*sin(phi), sqrt(max(1-rand[1], 0.0)));
}

void ComputeTangetSpace(in vec3 normal, out vec3 up, out vec3 right)
{
    up = abs(normal.y) > 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
    right = normalize(cross(up, normal));
    up    = cross(normal, right);
}

void main()
{
    vec3 dir;

    vec3 irradiance = vec3(0.0);
    vec3 normal     = normalize(coords);

    vec3 up, right;
    ComputeTangetSpace(normal, up, right);
    mat3 tangentSpc = mat3(right, up, normal);

    // \todo: try cosine weighted disk sampling ==> montecarlo intergration

    for(int i = 0;  i < NUM_SAMPLES; ++i)
    {
        vec3 dir = tangentSpc*hemisphereCosineSample(hammersley2D(i, NUM_SAMPLES));

        irradiance += texture(skybox, dir).rgb;
    }


    frag_color = vec4(irradiance/float(NUM_SAMPLES), 1.0);
}

