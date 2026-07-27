#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aTangent;
layout (location = 3) in vec2 aTexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uLightSpaceMatrix;
uniform mat4 uSpotLightSpaceMatrix;

out vec3 vWorldPosition;
out vec3 vWorldNormal;
out vec3 vWorldTangent;
out vec2 vTexCoord;
out vec4 vLightSpacePosition;
out vec4 vSpotLightSpacePosition;

void main()
{
    vec4 worldPosition = uModel * vec4(aPosition, 1.0);
    vWorldPosition = worldPosition.xyz;

    mat3 normalMatrix = mat3(transpose(inverse(uModel)));
    vWorldNormal = normalMatrix * aNormal;
    vWorldTangent = normalMatrix * aTangent;
    vTexCoord = aTexCoord;
    vLightSpacePosition = uLightSpaceMatrix * worldPosition;
    vSpotLightSpacePosition = uSpotLightSpaceMatrix * worldPosition;
    gl_Position = uProjection * uView * worldPosition;
}
