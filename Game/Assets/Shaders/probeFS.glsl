#version 460

uniform samplerCube cubemap;

in vec3 coords;
in vec3 position;

out vec4 color;

#ifdef PREFILTERED
uniform float roughness;
uniform int prefilteredLevels;
uniform sampler2D environmentBRDF;
uniform mat4 view;
#endif 


void main()
{
#ifdef PREFILTERED
    vec3 view_pos = transpose(mat3(view))*-view[3].xyz;
    vec3 V        = normalize(view_pos-position);
    float NdotV   = dot(normalize(coords), V);
    vec3 radiance = textureLod(cubemap, coords, roughness*(prefilteredLevels-1)).rgb;
    vec2 fab      = texture(environmentBRDF, vec2(NdotV, roughness)).rg;
    color         = vec4(radiance*(fab.x+fab.y), 1.0);
#else
    color = texture(cubemap, coords);
#endif 
}

