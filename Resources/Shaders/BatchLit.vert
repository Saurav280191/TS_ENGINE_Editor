#version 330 core 
layout (location = 0) in vec3 a_Position;//WorldSpace
layout (location = 1) in vec3 a_Color;
layout (location = 2) in vec2 a_TexCoord;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 v_Color;
out vec2 v_TexCoord;

void main()
{
    gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0f);    
    v_Color = a_Color;
    v_TexCoord = a_TexCoord;  
}