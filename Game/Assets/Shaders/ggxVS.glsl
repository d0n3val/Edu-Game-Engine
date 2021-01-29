#version 440

uniform mat4 matVP;
uniform mat4 matGeo;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 vtx_normal;
layout (location = 2) in vec2 texcoord;

out vec2 uv;
out vec3 normal;
out vec3 position;

void main() 
{
   uv = vec2(texcoord.x, 1.0-texcoord.y);
   normal = transpose(inverse(mat3(matGeo)))*vtx_normal;
   position = (matGeo*vec4(pos, 1)).xyz;
   gl_Position = matVP * matGeo * vec4(pos, 1);
}
