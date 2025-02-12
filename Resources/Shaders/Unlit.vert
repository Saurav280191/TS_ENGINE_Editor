#version 450 core
layout (location = 0) in vec4 a_Position;   // Position in ModelSpace
layout (location = 1) in vec2 a_TexCoord;   // UV
//layout (location = 2) in vec3 a_Normal;    	// Normal
layout (location = 3) in ivec4 a_BoneIds;   // Bone IDs
layout (location = 4) in vec4 a_Weights;    // Weights

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

// Model's root node's Id
uniform int u_ModelRootNodeId = -1;			// Sets after model spawns.										
uniform int u_SelectedModelRootNodeId = -1;	// Sets on bone selection.
uniform int u_SelectedBoneId = 0;			// Sets on bone selection.
uniform int u_BoneInfluence = 0;			// Sets on bone selection.

out vec2 v_TexCoord;
out vec4 o_InfluenceColor;

// Function to map a weight to a heatmap color
vec4 WeightToHeatmapColor(float weight) 
{
    weight = clamp(weight, 0.0, 1.0); // Ensure weight is between 0 and 1
    
    if (weight <= 0.25) 
	{
        // Blue to Cyan
        return vec4(0.0, weight * 4.0, 1.0, 1.0);
    } 
	else if (weight <= 0.5) 
	{
        // Cyan to Green
        return vec4(0.0, 1.0, 1.0 - (weight - 0.25) * 4.0, 1.0);
    } 
	else if (weight <= 0.75) 
	{
        // Green to Yellow
        return vec4((weight - 0.5) * 4.0, 1.0, 0.0, 1.0);
    } 
	else 
	{
        // Yellow to Red
        return vec4(1.0, 1.0 - (weight - 0.75) * 4.0, 0.0, 1.0);
    }
}

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
		if(u_SelectedModelRootNodeId == u_ModelRootNodeId 
		&& a_BoneIds[i] == u_SelectedBoneId)
		{
			// Change the color of the vertex if boneInfluence is enabled
			if(u_BoneInfluence == 1)
			{	
				// Red to Green
				o_InfluenceColor = WeightToHeatmapColor(a_Weights[i]);
			}
		}	
		
        hasBoneInfluence = true;
		
        vec4 localPosition = finalBonesMatrices[a_BoneIds[i]] * a_Position;
        totalPosition += localPosition * a_Weights[i];
    }
   
    gl_Position = (u_Projection * u_View * u_Model) * (hasBoneInfluence ? totalPosition : a_Position); 
	v_TexCoord = a_TexCoord;
}