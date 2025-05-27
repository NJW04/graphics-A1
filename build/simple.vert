#version 330 core

layout(location=0) in vec3 inPos;
layout(location=1) in vec2 inUV;
layout(location=2) in vec3 inNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos1;
uniform vec3 lightPos2;

out vec2  TexCoord;
out vec3  FragPos;
out vec3  Normal;
out vec3  L1;
out vec3  L2;

void main() {
    vec4 viewPos = view * model * vec4(inPos, 1.0);
    FragPos = viewPos.xyz;
    Normal  = normalize(mat3(view*model) * inNormal);

    vec3 lp1 = (view * vec4(lightPos1,1.0)).xyz;
    vec3 lp2 = (view * vec4(lightPos2,1.0)).xyz;
    L1 = lp1 - FragPos;
    L2 = lp2 - FragPos;

    TexCoord    = inUV;
    gl_Position = projection * viewPos;
}
