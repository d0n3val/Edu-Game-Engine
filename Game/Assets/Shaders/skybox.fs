#version 440 

out vec4 frag_color;

in vec3 coords;

uniform samplerCube skybox;

#ifdef USE_LOD
uniform float lod;
#endif 

void main()
{
#ifdef USE_LOD
    frag_color = textureLod(skybox, coords, lod);
#else
    frag_color = texture(skybox, coords);
#endif 
}
