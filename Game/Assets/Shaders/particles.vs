
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv0;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

struct Sheet
{
    int   x_tiles;
    int   y_tiles;
    float current;
};

layout(location=10) uniform Sheet sheet;

struct VertexOut
{
    vec2 uv0;
    vec2 uv1;
    float lambda;
    vec3 position;
};

out VertexOut fragment;

void GetSheetUV(out vec2 uv, in vec2 src_uv, in float current, in float x_tiles, in float y_tiles)
{
    float v0_pos = y_tiles-1.0-trunc(current/x_tiles);
    float u0_pos = trunc(mod(current, x_tiles));
    
    uv.x = mix(u0_pos, u0_pos+1, src_uv.x)/x_tiles;
    uv.y = mix(v0_pos, v0_pos+1, src_uv.y)/y_tiles;
}


void main()
{
    fragment.position = (model*vec4(vertex_position, 1.0)).xyz;

    GetSheetUV(fragment.uv0, vertex_uv0, sheet.current, sheet.x_tiles, sheet.y_tiles);
    GetSheetUV(fragment.uv1, vertex_uv0, sheet.current+1.0, sheet.x_tiles, sheet.y_tiles);

    fragment.lambda = sheet.current-trunc(sheet.current);

    gl_Position = proj*view*vec4(fragment.position, 1.0);
}
