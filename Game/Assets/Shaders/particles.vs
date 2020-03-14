
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv0;

layout(location = 3) in vec3 instance_right;
layout(location = 4) in vec3 instance_up;
layout(location = 5) in vec3 instance_front;
layout(location = 6) in vec3 instance_translation;
layout(location = 7) in vec4 instance_color;
layout(location = 8) in float instance_frame;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

struct Sheet
{
    int   x_tiles;
    int   y_tiles;
};

layout(location=10) uniform Sheet sheet;

struct VertexOut
{
    vec2 uv0;
    vec2 uv1;
    float lambda;
    vec3 position;
    vec4 color;
};

out VertexOut fragment;

void GetSheetUV(out vec2 uv, in vec2 src_uv, in float current, in float x_tiles, in float y_tiles)
{
    float v0_pos = (y_tiles-1.0)-trunc(current/x_tiles); // (y_tiles-1.0) --> flip v
    float u0_pos = trunc(mod(current, x_tiles));
    
    uv.x = mix(u0_pos, u0_pos+1, src_uv.x)/x_tiles;
    uv.y = mix(v0_pos, v0_pos+1, src_uv.y)/y_tiles;
}


void main()
{
    mat4 transform = mat4(vec4(instance_right, 0.0), vec4(instance_up, 0.0), vec4(instance_front, 0.0), vec4(instance_translation, 1.0));

    fragment.position = ((model*transform)*vec4(vertex_position, 1.0)).xyz;

    GetSheetUV(fragment.uv0, vertex_uv0, instance_frame, sheet.x_tiles, sheet.y_tiles);
    GetSheetUV(fragment.uv1, vertex_uv0, instance_frame+1.0, sheet.x_tiles, sheet.y_tiles);

    fragment.lambda = instance_frame-trunc(instance_frame);
    fragment.color = instance_color;

    gl_Position = proj*view*vec4(fragment.position, 1.0);
}
