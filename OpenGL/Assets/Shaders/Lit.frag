#version 330 core

#define MAX_POINT_LIGHTS 4

struct Material {
    vec3 albedo;
    vec3 specularColor;
    float shininess;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
    int enabled;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float innerCutoff;
    float outerCutoff;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    int enabled;
};

struct HemisphereLight {
    vec3 skyColor;
    vec3 groundColor;
    float intensity;
    vec3 bounceColor;
    float bounceStrength;
    float floorHeight;
    float bounceFalloff;
};

in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec4 vLightSpacePosition;

out vec4 fragmentColor;

uniform vec3 uViewPosition;
uniform Material uMaterial;
uniform DirectionalLight uDirectional;
uniform PointLight uPointLights[MAX_POINT_LIGHTS];
uniform int uPointLightCount;
uniform SpotLight uSpot;
uniform HemisphereLight uHemisphere;
uniform sampler2D uDirectionalShadowMap;

float evaluateDirectionalShadow(vec3 normal, vec3 lightDirection)
{
    vec3 projected = vLightSpacePosition.xyz / vLightSpacePosition.w;
    projected = projected * 0.5 + 0.5;

    if (projected.z > 1.0 ||
        projected.x < 0.0 || projected.x > 1.0 ||
        projected.y < 0.0 || projected.y > 1.0) {
        return 0.0;
    }

    float bias = max(
        0.0025 * (1.0 - dot(normal, lightDirection)),
        0.0005);
    vec2 texelSize = 1.0 / vec2(textureSize(uDirectionalShadowMap, 0));
    float shadow = 0.0;

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closestDepth = texture(
                uDirectionalShadowMap,
                projected.xy + vec2(x, y) * texelSize).r;
            shadow += projected.z - bias > closestDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

vec3 evaluateSurfaceLight(
    vec3 lightDirection,
    vec3 radiance,
    vec3 normal,
    vec3 viewDirection)
{
    float diffuseFactor = max(dot(normal, lightDirection), 0.0);
    vec3 halfwayDirection = normalize(lightDirection + viewDirection);
    float specularFactor = pow(
        max(dot(normal, halfwayDirection), 0.0),
        uMaterial.shininess);

    vec3 diffuse = uMaterial.albedo * diffuseFactor;
    vec3 specular = uMaterial.specularColor * specularFactor;
    return (diffuse + specular) * radiance;
}

vec3 evaluateDirectional(vec3 normal, vec3 viewDirection)
{
    if (uDirectional.enabled == 0) {
        return vec3(0.0);
    }

    vec3 lightDirection = normalize(-uDirectional.direction);
    vec3 radiance = uDirectional.color * uDirectional.intensity;
    float shadow = evaluateDirectionalShadow(normal, lightDirection);
    return (1.0 - shadow) *
        evaluateSurfaceLight(lightDirection, radiance, normal, viewDirection);
}

vec3 evaluatePoint(
    PointLight light,
    vec3 normal,
    vec3 viewDirection)
{
    vec3 toLight = light.position - vWorldPosition;
    float distanceToLight = length(toLight);
    vec3 lightDirection = toLight / max(distanceToLight, 0.0001);
    float attenuation = 1.0 / max(
        light.constantAttenuation +
        light.linearAttenuation * distanceToLight +
        light.quadraticAttenuation * distanceToLight * distanceToLight,
        0.0001);
    vec3 radiance = light.color * light.intensity * attenuation;
    return evaluateSurfaceLight(lightDirection, radiance, normal, viewDirection);
}

vec3 evaluateSpot(vec3 normal, vec3 viewDirection)
{
    if (uSpot.enabled == 0) {
        return vec3(0.0);
    }

    vec3 toLight = uSpot.position - vWorldPosition;
    float distanceToLight = length(toLight);
    vec3 lightDirection = toLight / max(distanceToLight, 0.0001);
    float coneAngle = dot(lightDirection, normalize(-uSpot.direction));
    float cone = clamp(
        (coneAngle - uSpot.outerCutoff) /
        max(uSpot.innerCutoff - uSpot.outerCutoff, 0.0001),
        0.0,
        1.0);
    float attenuation = 1.0 / max(
        uSpot.constantAttenuation +
        uSpot.linearAttenuation * distanceToLight +
        uSpot.quadraticAttenuation * distanceToLight * distanceToLight,
        0.0001);
    vec3 radiance = uSpot.color * uSpot.intensity * attenuation * cone;
    return evaluateSurfaceLight(lightDirection, radiance, normal, viewDirection);
}

vec3 evaluateIndirect(vec3 normal)
{
    float hemisphereBlend = normal.y * 0.5 + 0.5;
    vec3 hemisphere = mix(
        uHemisphere.groundColor,
        uHemisphere.skyColor,
        hemisphereBlend) * uHemisphere.intensity;

    float heightAboveFloor = max(vWorldPosition.y - uHemisphere.floorHeight, 0.0);
    float floorFacing = max(
        (dot(normal, vec3(0.0, -1.0, 0.0)) + 0.35) / 1.35,
        0.0);
    float bounceAttenuation =
        1.0 / (1.0 + heightAboveFloor * uHemisphere.bounceFalloff);
    vec3 floorBounce =
        uHemisphere.bounceColor *
        uHemisphere.bounceStrength *
        floorFacing *
        bounceAttenuation;

    return uMaterial.albedo * (hemisphere + floorBounce);
}

void main()
{
    vec3 normal = normalize(vWorldNormal);
    vec3 viewDirection = normalize(uViewPosition - vWorldPosition);

    vec3 lighting = evaluateIndirect(normal);
    lighting += evaluateDirectional(normal, viewDirection);

    for (int index = 0; index < uPointLightCount; ++index) {
        lighting += evaluatePoint(uPointLights[index], normal, viewDirection);
    }

    lighting += evaluateSpot(normal, viewDirection);

    vec3 mappedColor = lighting / (lighting + vec3(1.0));
    vec3 gammaCorrected = pow(mappedColor, vec3(1.0 / 2.2));
    fragmentColor = vec4(gammaCorrected, 1.0);
}
