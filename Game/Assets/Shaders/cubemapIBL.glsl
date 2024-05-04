#version 440 

#define ENVBRDF_NUM_SAMPLES 2048
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

// https://developer.nvidia.com/gpugems/gpugems3/part-iii-rendering/chapter-20-gpu-based-importance-sampling
// https://cgg.mff.cuni.cz/~jaroslav/papers/2007-sketch-fis/Final_sap_0073.pdf
float computeLod(float pdf, int numSamples, int width)
{
    // // note that 0.5 * log2 is equivalent to log4
    return max(0.5 * log2( 6.0 * float(width) * float(width) / (float(numSamples) * pdf)), 0.0);
}

#ifdef DIFFUSE_IBL

uniform int numSamples;
uniform int cubemapSize;
uniform int lodBias;

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

    for(int i = 0;  i < numSamples; ++i)
    {
        vec3 dir  = hemisphereCosineSample(hammersley2D(i, numSamples));
        float pdf = dir.z/PI;
        float lod = computeLod(pdf, numSamples, cubemapSize);

        dir.xyz   = tangentSpace*dir.xyz;

        irradiance += textureLod(skybox, dir.xyz, lod+lodBias).rgb;
    }

    fragColor = vec4(irradiance/float(numSamples), 1.0);
}

#else

vec3 hemisphereSampleGGX(in vec2 rand, float roughness)
{
    float a = roughness*roughness;
    float phi = 2.0*PI*rand.x;
    float cos_theta = sqrt((1.0-rand.y)/(rand.y*(a*a-1)+1));
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
uniform int numSamples;
uniform int cubemapSize;
uniform int lodBias;

float Sq(float value) {return value*value;}

float D_GGX(float NdotH, float roughness) 
{
    float roughnessSq = roughness*roughness;
    return roughnessSq/(PI*Sq(Sq(NdotH)*(roughnessSq-1)+1));
}

void main()
{
    vec3 R = normalize(coords);
    vec3 N = R, V = R;

    vec3 color        = vec3(0.0);
    float weight      = 0.0;
    mat3 tangentSpace = computeTangetSpace(N);

    for( int i = 0; i < numSamples; ++i ) 
    {
        vec3 dir = hemisphereSampleGGX( hammersley2D(i, numSamples), roughness);

        // float pdf = D_GGX(NoH, roughness) * NoH / (4.0 * VoH);
        // but since V = N => VoH == NoH
        float pdf = D_GGX(dir.z, roughness)/4.0;
        float lod = computeLod(pdf, numSamples, cubemapSize);

        vec3 H = normalize(tangentSpace*dir);
        vec3 L = reflect(-V, H); 
        float NdotL = dot( N, L );
        if( NdotL > 0 ) 
        {
            color += textureLod(skybox, L, lod+lodBias).rgb * NdotL;
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
    return 0.5 / (GGXV + GGXL);
    float roughnessSq = roughness * roughness;
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

    for (uint i = 0; i < ENVBRDF_NUM_SAMPLES; i++) 
    {
        vec3 H = hemisphereSampleGGX(hammersley2D(i, ENVBRDF_NUM_SAMPLES), roughness);

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

    fragColor = vec4(4.0*fa/float(ENVBRDF_NUM_SAMPLES), 4.0*fb/float(ENVBRDF_NUM_SAMPLES), 1.0, 1.0);
}

#endif 

#endif 


