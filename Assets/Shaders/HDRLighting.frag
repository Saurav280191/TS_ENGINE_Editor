#version 450 core

out vec4 v_FragColor;
out uint entityID;

in vec3 v_FragPos;//World space
in vec3 v_Color;
in vec3 v_Normal;
in vec2 v_TexCoord;

uniform sampler2D u_Sampler;
uniform int u_HasTexture;
uniform vec3 u_ViewPos;
uniform vec3 u_LightPos;
uniform bool u_Gamma;
uniform float u_GammaValue;
uniform bool u_Hdr;
uniform float u_HdrExposure;
uniform int u_EntityID;

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform DirLight dirLight;

float shininess = 16.0f;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 color, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    
    // combine results
    vec3 ambient  = light.ambient * color;
    vec3 diffuse  = light.diffuse * diff * color;
    vec3 specular = light.specular * spec * color;

    float max_distance = 1.5;
    float distance = length(u_LightPos - v_FragPos);
    //float attenuation = 1.0 / (u_Gamma ? distance * distance : distance);
    
    //diffuse *= attenuation;
    //specular *= attenuation;

    return (ambient + diffuse + specular);
} 

void main()
{ 
    vec4 result;

    if(u_HasTexture == 1)
    {
        result = vec4(v_Color, 1) * texture(u_Sampler, v_TexCoord);
    }   
    else
    {
        result = vec4(v_Color, 1);
    }

    //vec3 viewDir = normalize(u_ViewPos - v_FragPos);
    //result = CalcDirLight(dirLight, v_Normal, result, viewDir);

    //if(u_Hdr)
    //{
    //    //Tone mapping
    //    result *= vec3(1.0) - exp(-result * u_HdrExposure);//Exposure           
    //    
    //    if(u_Gamma)
    //        result = pow(result, vec3(1.0 / u_GammaValue));        
    //}
    //else
    //{
    //    if(u_Gamma)
    //        result = pow(result, vec3(1.0 / u_GammaValue));
    //}

    v_FragColor = result;
    entityID = u_EntityID;
}    