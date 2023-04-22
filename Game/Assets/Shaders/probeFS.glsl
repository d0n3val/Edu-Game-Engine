#version 460

uniform samplerCube cubemap;

in vec3 coords;

out vec4 color;

#ifdef USE_LOD
uniform float lod;
#endif 


void main()
{
#ifdef USE_LOD
    color = textureLod(cubemap, coords, lod);
#else
    color = texture(cubemap, coords);
#endif 
}

