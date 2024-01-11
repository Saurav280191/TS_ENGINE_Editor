#version 450 core
layout (location = 0) in vec4 a_Position;// ModelSpace
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in vec3 a_Normal;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 v_FragPos;
out vec2 v_TexCoord;
out vec3 v_Normal;

void main()
{
    v_FragPos = vec3(u_Model * a_Position);// World space
    v_TexCoord = a_TexCoord;
	v_Normal = a_Normal;

    gl_Position = u_Projection * u_View * u_Model * a_Position;  
}