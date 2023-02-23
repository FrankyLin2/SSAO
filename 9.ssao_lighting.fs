#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;
uniform sampler2D gDepth;
struct Light {
    // vec3 Position;
    vec3 direction;
    vec3 Color;
    float Linear;
    float Quadratic;
};

uniform Light light;

float rand(vec2 value, float x, vec2 y){
    //avoid overflew float limit
    vec2 smallValue = sin(value);
    //get random
    float random = dot(smallValue, y);
    random = fract(sin(random) * x);
    return random;
}

void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb + 0.3 * (rand(TexCoords, 14375.5964, vec2(15.637, 76.243))-0.5);
    float AmbientOcclusion = texture(ssao, TexCoords).r;
    
    // then calculate lighting as usual
    vec3 ambient = vec3(0.4 * Diffuse * AmbientOcclusion);
    vec3 lighting  = ambient; 
    vec3 viewDir  = normalize(-FragPos); // viewpos is (0.0.0)
    // diffuse
    vec3 lightDir = normalize(light.direction);
    // vec3 diffuse = max(dot(Normal, light.direction), 0.0) * Diffuse * light.Color;
    vec3 diffuse = (2.0 * (1-pow(dot(Normal,lightDir),2))+vec3(0.2)) * ambient * light.Color;
    // // specular
    // vec3 halfwayDir = normalize(light.direction + viewDir);  
    // float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    // vec3 specular = light.Color * spec;
    // // attenuation
    // float distance = length(vec3(0.0f) - vec3(0.0, 0.0, FragPos.z));
    // float attenuation = 1.0 / (1.0 + light.Linear * distance + light.Quadratic * distance * distance);
    // diffuse *= attenuation;
    // specular *= attenuation;
    lighting += diffuse;

    FragColor = vec4(lighting, 1.0);

    // float depth = texture(gDepth,TexCoords).r;
    // vec3 depthcolor = vec3(depth);
    // FragColor = vec4(depthcolor, 1.0);
}
