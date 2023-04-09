#version 330 core
in vec3 v_Color;
in vec2 v_TexCoord;

uniform sampler2D u_Sampler;

out vec4 v_FragColor;

void main()
{ 
    vec4 texCol = texture(u_Sampler, v_TexCoord); 
    v_FragColor = vec4(v_Color, 1) * texCol;
}        