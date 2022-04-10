#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in mat4 aInstanceMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec3 TangentLightPos;
out vec3 TangentLightDir;
out vec3 TangentViewPos;
out vec3 TangentFragPos;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 lightDir;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    FragPos = vec3(aInstanceMatrix * vec4(aPos, 1.0));
    TexCoords = aTexCoords;

    mat3 normalMatrix = transpose(inverse(mat3(aInstanceMatrix)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    mat3 TBN = transpose(mat3(T, B, N));
    TangentLightPos = TBN * lightPos;

    TangentViewPos = TBN * viewPos;
    TangentFragPos = TBN * FragPos;

    Normal = aNormal;

    TangentLightDir = TBN * lightDir;

    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0);
}
