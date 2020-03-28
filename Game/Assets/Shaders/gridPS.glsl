uniform mat4 view;
in vec2 posXY;
out vec4 outColor;

void main() 
{
   vec3 view_pos           = transpose(mat3(view))*-view[3].xyz;
   float cell_size_lod0    = log(view_pos.y/20+1)/log(10.0);

   float lod_change_amount = fract(cell_size_lod0);
   cell_size_lod0          = floor(cell_size_lod0);
   
   float der_x             = max(abs(dFdx(posXY.x)), abs(dFdy(posXY.x)));
   float der_y 			   = max(abs(dFdx(posXY.y)), abs(dFdy(posXY.y)));
        
   vec2 dist_to_line_lod0  = abs(mod(posXY, pow(10, cell_size_lod0)));
   vec2 dist_to_line_lod1  = abs(mod(posXY, pow(10, cell_size_lod0+1)));
   vec2 dist_to_line_lod2  = abs(mod(posXY, pow(10, cell_size_lod0+2)));
   
   vec3 color = vec3(0.0);
   float alpha = 1.0;

   if(dist_to_line_lod2.x < der_x || dist_to_line_lod2.y < der_y)
   {
       color = vec3(0.7);       
       alpha = 1.0; 
   }
   else if(dist_to_line_lod1.x < der_x || dist_to_line_lod1.y < der_y)
   {
       color = vec3(0.7+(0.3-0.7)*lod_change_amount);       
       alpha = 1.0; 
   }
   else if(dist_to_line_lod0.x < der_x || dist_to_line_lod0.y < der_y)
   {
        color = vec3(0.3);
        alpha = 1.0-lod_change_amount;
   }
   else
   {
       discard;
   }

   outColor = vec4(color, alpha);
}
