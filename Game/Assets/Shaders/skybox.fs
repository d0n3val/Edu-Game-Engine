#version 440 

out vec4 frag_color;

in vec3 coords;

uniform samplerCube skybox;

void main()
{
    frag_color = texture(skybox, coords);
}
