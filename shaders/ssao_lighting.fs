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
    float ka;
    float kd;
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
    vec3 Diffuse = texture(gAlbedo, TexCoords).rgb;
    float AmbientOcclusion = texture(ssao, TexCoords).r;
    
    // then calculate lighting as usual
    vec3 ambient = vec3(light.ka * Diffuse * AmbientOcclusion);
    // vec3 lighting  = ambient; 
    vec3 viewDir  = normalize(-FragPos); // viewpos is (0.0.0)
    // diffuse
    vec3 lightDir = normalize(light.direction);
    float t = clamp((AmbientOcclusion*255-150)/50, 0.0, 1.0 );
    //做一个blending
    vec3 diffuse = light.kd * ( (1-t)* ((1-dot(Normal,lightDir) * dot(Normal,lightDir))+0.2) * Diffuse + vec3(AmbientOcclusion+0.3)*t );
    // vec3 diffuse =  light.kd * ((1-dot(Normal,lightDir) * dot(Normal,lightDir))) * ambient * light.Color;
    vec3 lighting = ambient + diffuse + 0.3 * (rand(TexCoords, 14375.5964, vec2(15.637, 76.243))-0.5);

    // 实际输出
    FragColor = vec4(lighting, 1.0);

//  
    //检查SSAO
    // FragColor = vec4(AmbientOcclusion, AmbientOcclusion, AmbientOcclusion, 1.0);
    // FragColor = vec4(diffuse, 1.0);
    // FragColor = vec4(t, t, t, 1.0);
    //检查深度图
    // float depth = texture(gDepth,TexCoords).r;
    // vec3 depthcolor = vec3(depth);
    // FragColor = vec4(depthcolor, 1.0);
}
