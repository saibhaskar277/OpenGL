#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uLightSpaceMatrix;

out vec3 vWorldPosition;
out vec3 vWorldNormal;
out vec4 vLightSpacePosition;

void main()
{
    vec4 worldPosition = uModel * vec4(aPosition, 1.0);
    vWorldPosition = worldPosition.xyz;
    vWorldNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vLightSpacePosition = uLightSpaceMatrix * worldPosition;
    gl_Position = uProjection * uView * worldPosition;
}
