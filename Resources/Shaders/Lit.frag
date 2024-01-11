#version 450 core

out vec4 v_FragColor;
out uint entityID;// Only for editor

in vec3 v_FragPos;// World space
in vec2 v_TexCoord;
in vec3 v_Normal;

uniform vec4 u_AmbientColor;

uniform vec4 u_DiffuseColor;
uniform int u_HasDiffuseTexture;
uniform vec2 u_DiffuseMapOffset;
uniform vec2 u_DiffuseMapTiling;
uniform sampler2D u_DiffuseSampler;

uniform vec4 u_SpecularColor;
uniform int u_HasSpecularTexture;
uniform vec2 u_SpecularMapOffset;
uniform vec2 u_SpecularMapTiling;
uniform sampler2D u_SpecularSampler;

uniform int u_HasNormalTexture;
uniform vec2 u_NormalMapOffset;
uniform vec2 u_NormalMapTiling;
uniform sampler2D u_NormalSampler;

uniform vec3 u_ViewPos;
uniform vec3 u_LightPos;

uniform int u_EntityID;// Only for editor

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirLight dirLight;

//float shininess = 16.0f;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 color, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // specular shading
    //vec3 reflectDir = reflect(-lightDir, normal);
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    
    // combine results
    vec3 ambient  = light.ambient * color;
    vec3 diffuse  = light.diffuse * diff * color;
    //vec3 specular = light.specular * spec * color;

    float max_distance = 1.5;
    float distance = length(u_LightPos - v_FragPos);
    float attenuation = 1.0 / distance;
    //float attenuation = 1.0 / (u_Gamma ? distance * distance : distance);
	
    diffuse *= attenuation;
    //specular *= attenuation;

    return (ambient + diffuse);// + specular);
} 

void main()
{ 
	// Ambient
    vec4 result = u_AmbientColor;    
    result *= u_DiffuseColor;
	
	// Diffuse
    if(u_HasDiffuseTexture == 1)
    {
		vec2 tiledAndOffsetTexCoords = (v_TexCoord * u_DiffuseMapTiling) + (u_DiffuseMapOffset * 0.01f);
        result *= texture2D(u_DiffuseSampler, tiledAndOffsetTexCoords);
    }

    //vec3 viewDir = normalize(u_ViewPos - v_FragPos);
    //result = vec4(CalcDirLight(dirLight, v_Normal, vec3(result.x, result.y, result.z), viewDir), 1);

    v_FragColor = result;
    entityID = u_EntityID;
}    