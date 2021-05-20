#version 440

#define PI 3.141597

out vec4 color;

in vec3 coords;

uniform sampler2D skybox;

vec2 CartesianToEquirectangular(in vec3 dir)
{
    float phi;

    phi = atan(dir.z, dir.x); // between -PI , PI
    phi = phi/(2.0*PI)+0.5;

    float theta = asin(dir.y);  // between -PI/ ,  PI/2
    theta = theta/PI+0.5;

    return vec2(phi, theta);
}

void main()
{
    vec3 dir = normalize(coords);
    vec2 uv  = CartesianToEquirectangular(dir);
    color    = texture(skybox, uv);
}

