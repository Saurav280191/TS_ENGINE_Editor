#version 450 core
layout (location = 0) in vec4 a_Position;	// Position in ModelSpace
layout (location = 1) in vec2 a_TexCoord;	// UV
layout (location = 4) in ivec4 a_BoneIds;	// iVec4
layout (location = 5) in vec4 a_Weights;	// fVec4

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 v_TexCoord;

void main()
{
	vec4 finalVertPos = vec4(0.0f);	// FinalVertPos is the vertex position after influenced by all bones 
	
	for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
	{
		if(a_BoneIds[i] == -1)
			continue;
		
		// Break after MAX_BONES
		if(a_BoneIds[i] >= MAX_BONES)
		{
			finalVertPos = a_Position;
			break;
		}
		
		vec4 localPosition = finalBonesMatrices[a_BoneIds[i]] * a_Position;
		finalVertPos += localPosition * a_Weights[i];
	}
	
    gl_Position = (u_Projection * u_View * u_Model) * finalVertPos;	// Position in Clip space   
	v_TexCoord = a_TexCoord;										// UV
}