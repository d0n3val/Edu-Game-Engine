#version 440 

#define NUM_SAMPLES 1024
#define PI 3.1415926
#define TWO_PI 2.0*PI
#define HALF_PI 0.5*PI

out vec4 fragColor;

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

mat3 computeTangetSpace(in vec3 normal)
{
    vec3 up    = abs(normal.y) > 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, normal));
    up         = cross(normal, right);

    return mat3(right, up, normal);
}

#ifdef DIFFUSE_IBL

vec3 hemisphereCosineSample(in vec2 rand)
{
     float phi = rand[0] * 2.0 * PI;
     float r   = sqrt(rand[1]);

     return vec3(r*cos(phi), r*sin(phi), sqrt(max(1-rand[1], 0.0)));
}

void main()
{
    vec3 dir;

    vec3 irradiance   = vec3(0.0);
    vec3 normal       = normalize(coords);
    mat3 tangentSpace = computeTangetSpace(normal);

    for(int i = 0;  i < NUM_SAMPLES; ++i)
    {
        vec3 dir = tangentSpace*hemisphereCosineSample(hammersley2D(i, NUM_SAMPLES));

        irradiance += texture(skybox, dir).rgb;
    }

    fragColor = vec4(irradiance/float(NUM_SAMPLES), 1.0);
}

#else

vec3 hemisphereSampleGGX(in vec2 rand, float roughness)
{
    float phi = 2.0*PI*rand.x;
    float cos_theta = sqrt((1.0-rand.y)/(rand.y*(roughness*roughness-1)+1));
    float sin_theta = sqrt(1-cos_theta*cos_theta);

    // spherical to cartesian conversion
    vec3 dir;
    dir.x = cos(phi)*sin_theta;
    dir.y = sin(phi)*sin_theta;
    dir.z = cos_theta;

    return dir;
}

#ifdef PREFILTERED_IBL

uniform float roughness;

void main()
{
    vec3 R = normalize(coords);
    vec3 N = R, V = R;

    vec3 color        = vec3(0.0);
    float weight      = 0.0;
    mat3 tangentSpace = computeTangetSpace(N);

    for( int i = 0; i < NUM_SAMPLES; ++i ) 
    {
        vec3 H = tangentSpace*hemisphereSampleGGX( hammersley2D(i, NUM_SAMPLES), roughness);
        vec3 L = reflect(-V, H); 
        float NdotL = dot( N, L );
        if( NdotL > 0 ) 
        {
            color += texture(skybox, L).rgb * NdotL;
            weight += NdotL;
        }
    }

    fragColor = vec4(color / weight, 1.0);
}

#else  // ENVIRONMENT BRDF

in vec2 uv;

float SmithVSF(float NdotL, float NdotV, float roughness)
{
    float GGXV = NdotL * (NdotV * (1.0 - roughness) + roughness);
    float GGXL = NdotV * (NdotL * (1.0 - roughness) + roughness);
    return 0.5 / max(0.00001, (GGXV + GGXL));
}

void main()
{
    float NdotV = uv.x;
    float roughness = uv.y;

    vec3 V;
    V.x = sqrt(1.0 - NdotV * NdotV); // sin
    V.y = 0.0;
    V.z = NdotV; // cos

    vec3 N = vec3(0.0, 0.0, 1.0);

    float fa = 0.0;
    float fb = 0.0;

    for (uint i = 0; i < NUM_SAMPLES; i++) 
    {
        vec3 H = hemisphereSampleGGX(hammersley2D(i, NUM_SAMPLES), roughness);

        // Get the light direction
        vec3 L = reflect(-V, H); 

        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if (NdotL > 0.0) 
        {
            float V_pdf = SmithVSF(NdotL, NdotV, roughness) * VdotH * NdotL / NdotH;
            float Fc = pow(1.0 - VdotH, 5.0); // note: VdotH = LdotH
            fa += (1.0 - Fc) * V_pdf;
            fb += Fc * V_pdf;
        }
    }

    fragColor = vec4(4.0*fa/float(NUM_SAMPLES), 4.0*fb/float(NUM_SAMPLES), 1.0, 1.0);
}

#endif 

#endif 


