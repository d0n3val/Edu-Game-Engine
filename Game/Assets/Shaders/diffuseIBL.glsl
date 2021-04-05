#version 440 

#define NUM_PHI_SAMPLES 256
#define NUM_THETA_SAMPLES 256
#define PI 3.1415926
#define TWO_PI 2.0*PI
#define HALF_PI 0.5*PI

out vec4 frag_color;

in vec3 coords;

uniform samplerCube skybox;

void ComputeTangetSpace(in vec3 normal, out vec3 up, out vec3 right)
{
    if(coords.x > coords.y || coords.z > coords.y)
    {
        up = vec3(0.0, 1.0, 0.0);
    }
    else
    {
        up = vec3(1.0, 0.0, 0.0);
    }
   
    right = cross(up, normal);
    up    = cross(normal, right);
}

void main()
{
    vec3 dir;

    vec3 irradiance = vec3(0.0);
    vec3 normal     = normalize(coords);

    vec3 up, right;
    ComputeTangetSpace(normal, up, right);

    // \todo: try cosine weighted disk sampling ==> montecarlo intergration

    for(int i = 0;  i < NUM_PHI_SAMPLES; ++i)
    {
        float phi = TWO_PI*2.0*float(i)/float(NUM_PHI_SAMPLES);

        for(int j = 0; j < NUM_THETA_SAMPLES; ++j)
        {
            float theta = HALF_PI*float(j)/float(NUM_THETA_SAMPLES);

            float sin_theta = sin(theta); 
            float cos_theta = cos(theta);

            dir = right*cos(phi)*sin_theta+normal*sin(phi)*sin_theta+up*cos_theta;

            vec3 Li = texture(skybox, dir).rgb;

            irradiance += Li*cos_theta*sin_theta;
        }
    }

    // \todo: sobra pi ???
    frag_color.rgb = PI*irradiance/float(NUM_PHI_SAMPLES*NUM_THETA_SAMPLES);
}

