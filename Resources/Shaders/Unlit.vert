#version 450 core
layout (location = 0) in vec4 a_Position;    // Position in ModelSpace
layout (location = 1) in vec2 a_TexCoord;    // UV
layout (location = 2) in vec3 a_Normal;      // Normal
layout (location = 3) in ivec4 a_BoneIds;    // Bone IDs
layout (location = 4) in vec4 a_Weights;     // Weights

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

uniform int selectedBoneId = 0;

out vec2 v_TexCoord;
out vec4 o_InfluenceColor;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    o_InfluenceColor = vec4(1.0f, 1.0f, 1.0f, 1.0f); // Default white
    bool hasBoneInfluence = false;

    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        // Skip invalid bones or zero weights
        if(a_BoneIds[i] >= MAX_BONES || a_Weights[i] <= 0.0) 
        {
			continue;
		}
		
		// Set bone influence color
		if(a_BoneIds[i] == selectedBoneId)
		{
			// Ensure it's within range
			float weight = clamp(a_Weights[i], 0.0, 1.0);
			// Red to Green
			o_InfluenceColor = vec4(weight, 1.0 - weight, 0.0, 1.0);
		}	
		
        hasBoneInfluence = true;
		
        vec4 localPosition = finalBonesMatrices[a_BoneIds[i]] * a_Position;
        totalPosition += localPosition * a_Weights[i];
    }
   

    gl_Position = (u_Projection * u_View * u_Model) 
	* (hasBoneInfluence ? totalPosition : a_Position);
    
	v_TexCoord = a_TexCoord;
}