#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out float depth;
out vec3 iAlbedo;
//in cube, need to invert normals inside.
uniform bool invertedNormals;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 Albedo;
void main()
{
    vec4 viewPos = view * model * vec4(aPos, 1.0);
    FragPos = viewPos.xyz; 
    TexCoords = aTexCoords;
    iAlbedo = Albedo.xyz;
    mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    Normal = normalMatrix * (invertedNormals ? -aNormal : aNormal);
    vec4 NDCPos = projection * viewPos;
    depth = (NDCPos.z+1)/2;
    gl_Position = projection * viewPos;
    
}