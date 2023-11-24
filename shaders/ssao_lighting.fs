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
    float kg;
};
uniform int OutputMode;
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
    vec3 Albedo = texture(gAlbedo, TexCoords).rgb;
    float AmbientOcclusion = texture(ssao, TexCoords).r;
    float Depth = texture(gDepth,TexCoords).r;
    // 形貌衬度
    vec3 ShapeContrast = vec3(light.kg * Albedo * AmbientOcclusion);

    vec3 lightDir = -normalize(light.direction);

    float t = clamp((abs(AmbientOcclusion - 0.5) - 0.05) / 0.2, 0.0, 1.0 ) / 2 + 0.5;
    //sin的diffuse
    // vec3 pureDiffuse = light.kd * (sqrt(1-dot(Normal,lightDir) * dot(Normal,lightDir))) * Albedo;

    //夹角衬度
    vec3 AngleContrast = light.ka * (1 - max(dot(Normal, lightDir), 0)) * Albedo;
    
    //做一个blending
    // vec3 diffuse =  light.kd * ((1-t) * ((1-dot(Normal,lightDir) * dot(Normal,lightDir))) + t * AmbientOcclusion)* Albedo; 
    vec3 result =   (1-t) * AngleContrast + t * ShapeContrast+ 0.3 * (rand(TexCoords, 14375.5964, vec2(15.637, 76.243))-0.5); 
    switch(OutputMode)
    {
        case 0:
            FragColor = vec4(result, 1.0);
            break;
        //不能很好可视化，因为FragPos是ViewSpace的，很大没有归一化
        case 1:
            FragColor = vec4((FragPos.xy/15 + 1 ) / 2, (FragPos.z / 200 + 1 ) / 2, 1.0);
            break;
        case 2:
            FragColor = vec4(Normal, 1.0);
            break;
        case 3:
            FragColor = vec4(Albedo, 1.0);
            break;
        //0,1之间的深度
        case 4:
            FragColor = vec4(Depth, Depth, Depth, 1.0);
            break;
        //AO
        case 5:
            FragColor = vec4(AmbientOcclusion, AmbientOcclusion, AmbientOcclusion, 1.0);
            break;
        case 6:
            FragColor = vec4(ShapeContrast, 1.0);
            break;
        case 7:
            FragColor = vec4(AngleContrast, 1.0);
            break;
        case 8:
            FragColor = vec4(t, t, t, 1.0);
            break;
        default:
            FragColor = vec4(result, 1.0);
            break;
    }
}
