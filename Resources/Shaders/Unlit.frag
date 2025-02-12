#version 450 core
out vec4 v_FragColor;
out uint nodeId;		// *** Required for picking ***

//in vec3 v_Normal;
in vec2 v_TexCoord;

uniform vec4 u_AmbientColor;

uniform vec4 u_DiffuseColor;
uniform int u_HasDiffuseTexture;
uniform vec2 u_DiffuseMapOffset;
uniform vec2 u_DiffuseMapTiling;
uniform sampler2D u_DiffuseSampler;

uniform int u_NodeId;	// *** Required for picking ***

in vec4 o_InfluenceColor;

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

    v_FragColor = result * o_InfluenceColor;
    nodeId = u_NodeId;
}    