#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out float gDepth;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in float depth;
void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // and the diffuse per-fragment color
    //to do: when object have texture,change to texture sample
    gAlbedo.rgb = vec3(0.2);
    gDepth = depth;
}