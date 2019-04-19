in vec4 frag_pos;

out vec4 color;

void main()
{
    float depth = frag_pos.z/frag_pos.w;
    depth = depth*0.5+0.5;

    float depth_2= depth*depth;

    float dx = dFdx(depth);
    float dy = dFdy(depth);
    depth_2 += 0.25*(dx*dx+dy*dy) ;

    color = vec4(depth, depth_2, 0.0, 1.0);
}
