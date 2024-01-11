#version 450 core

out vec4 v_FragColor;
out uint entityID;// Only for editor

in vec3 v_FragPos;// World space
in vec3 v_Normal;
in vec2 v_TexCoord;

uniform vec4 u_AmbientColor;

uniform vec4 u_DiffuseColor;
uniform int u_HasDiffuseTexture;
uniform vec2 u_DiffuseMapOffset;
uniform vec2 u_DiffuseMapTiling;
uniform sampler2D u_DiffuseSampler;

uniform int u_EntityID;// Only for editor

void main()
{ 
	// Ambient
    vec4 result = u_AmbientColor;    
	
	// Diffuse
    if(u_HasDiffuseTexture == 1)
    {
		vec2 tiledAndOffsetTexCoords = (v_TexCoord * u_DiffuseMapTiling) + (u_DiffuseMapOffset * 0.01f);
        result *= texture2D(u_DiffuseSampler, tiledAndOffsetTexCoords);
    }

    v_FragColor = result;
    entityID = u_EntityID;
}    