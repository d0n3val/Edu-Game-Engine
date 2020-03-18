uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

layout (location = 0) in vec3 pos;

out vec2 posXY;

void main() 
{ 
   gl_Position = proj* view * model * vec4(pos, 1);
   posXY = (model * vec4(pos, 1)).xz;
}
