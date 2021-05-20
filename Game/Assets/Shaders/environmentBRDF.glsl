#version 440 

#define NUM_SAMPLES 1024

out vec4 color;
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
        vec2 Xi = hammersley(i, numSamples);
        // Sample microfacet direction
        vec3 H = importanceSampleGGX(Xi, roughness, N);

        // Get the light direction
        vec3 L = reflect(-V, H); 

        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if (NoL > 0.0) 
        {
            // todo: check with my equation
            float V_pdf = SmithVSF(NdoL, NdotV, roughness) * VdotH * NdotL / NdotH;
            float Fc = pow(1.0 - VdotH, 5.0);
            fa += (1.0 - Fc) * V_pdf;
            fb += Fc * V_pdf;
        }
    }

    color =  vec2(fa, fb) / float(NUM_SAMPLES);
}

