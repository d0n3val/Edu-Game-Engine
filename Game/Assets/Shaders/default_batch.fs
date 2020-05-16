struct VertexOut
{
    vec2 uv0;
    vec3 normal;
    vec3 tangent;
    vec3 position;
};

in VertexOut fragment;
in vec4 shadow_coord[3];
out vec4 color;

void main()
{
    color = vec4(1.0);
}
