#version 330 core

#define MAX_POINT_LIGHTS 4

struct Material {
    vec3 albedo;
    vec3 specularColor;
    float shininess;
    vec3 emissive;
    float ambientOcclusion;
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
    vec3 upDirection;
    int enabled;
};

in vec3 vWorldPosition;
in vec3 vWorldNormal;
in vec3 vWorldTangent;
in vec2 vTexCoord;
in vec4 vLightSpacePosition;
in vec4 vSpotLightSpacePosition;

out vec4 fragmentColor;

uniform vec3 uViewPosition;
uniform Material uMaterial;
uniform DirectionalLight uDirectional;
uniform PointLight uPointLights[MAX_POINT_LIGHTS];
uniform int uPointLightCount;
uniform SpotLight uSpot;
uniform HemisphereLight uHemisphere;

uniform sampler2D uDirectionalShadowMap;
uniform sampler2D uSpotShadowMap;
uniform samplerCube uPointShadowMaps[MAX_POINT_LIGHTS];
uniform int uDirectionalCastsShadow;
uniform int uSpotCastsShadow;
uniform int uPointShadowCount;
uniform float uPointShadowFarPlane;

uniform sampler2D uAlbedoMap;
uniform sampler2D uNormalMap;
uniform sampler2D uSpecularMap;
uniform sampler2D uEmissiveMap;
uniform sampler2D uAoMap;
uniform int uUseAlbedoMap;
uniform int uUseNormalMap;
uniform int uUseSpecularMap;
uniform int uUseEmissiveMap;
uniform int uUseAoMap;

vec3 sampleAlbedo()
{
    vec3 color = uMaterial.albedo;
    if (uUseAlbedoMap == 1) {
        color *= texture(uAlbedoMap, vTexCoord).rgb;
    }
    return color;
}

vec3 sampleSpecular()
{
    vec3 color = uMaterial.specularColor;
    if (uUseSpecularMap == 1) {
        color *= texture(uSpecularMap, vTexCoord).rgb;
    }
    return color;
}

vec3 sampleEmissive()
{
    vec3 color = uMaterial.emissive;
    if (uUseEmissiveMap == 1) {
        color += texture(uEmissiveMap, vTexCoord).rgb;
    }
    return color;
}

float sampleAo()
{
    float ao = uMaterial.ambientOcclusion;
    if (uUseAoMap == 1) {
        ao *= texture(uAoMap, vTexCoord).r;
    }
    return ao;
}

vec3 sampleNormal()
{
    vec3 normal = normalize(vWorldNormal);
    if (uUseNormalMap == 0) {
        return normal;
    }

    vec3 tangent = normalize(vWorldTangent);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal);

    vec3 tangentNormal = texture(uNormalMap, vTexCoord).xyz * 2.0 - 1.0;
    return normalize(tbn * tangentNormal);
}

float sampleShadowMap2D(
    sampler2D shadowMap,
    vec4 lightSpacePosition,
    vec3 normal,
    vec3 lightDirection)
{
    vec3 projected = lightSpacePosition.xyz / lightSpacePosition.w;
    projected = projected * 0.5 + 0.5;

    if (projected.z > 1.0 ||
        projected.x < 0.0 || projected.x > 1.0 ||
        projected.y < 0.0 || projected.y > 1.0) {
        return 0.0;
    }

    float bias = max(
        0.0025 * (1.0 - dot(normal, lightDirection)),
        0.0005);
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float shadow = 0.0;

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closestDepth = texture(
                shadowMap,
                projected.xy + vec2(x, y) * texelSize).r;
            shadow += projected.z - bias > closestDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

float samplePointShadow(int lightIndex, vec3 lightPosition)
{
    if (lightIndex < 0 || lightIndex >= uPointShadowCount) {
        return 0.0;
    }

    vec3 fragToLight = vWorldPosition - lightPosition;
    float currentDepth = length(fragToLight);
    float closestDepth = 1.0;

    // GLSL 330: sampler arrays need effectively-constant indices.
    if (lightIndex == 0) {
        closestDepth = texture(uPointShadowMaps[0], fragToLight).r;
    } else if (lightIndex == 1) {
        closestDepth = texture(uPointShadowMaps[1], fragToLight).r;
    } else if (lightIndex == 2) {
        closestDepth = texture(uPointShadowMaps[2], fragToLight).r;
    } else {
        closestDepth = texture(uPointShadowMaps[3], fragToLight).r;
    }

    closestDepth *= uPointShadowFarPlane;
    float bias = 0.08;
    return currentDepth - bias > closestDepth ? 1.0 : 0.0;
}

// Blinn-Phong. lightDirection must point from the surface toward the light.
vec3 evaluateSurfaceLight(
    vec3 lightDirection,
    vec3 radiance,
    vec3 normal,
    vec3 viewDirection,
    vec3 albedo,
    vec3 specularColor)
{
    float nDotL = max(dot(normal, lightDirection), 0.0);
    if (nDotL <= 0.0) {
        return vec3(0.0);
    }

    vec3 halfwayDirection = normalize(lightDirection + viewDirection);
    float specularFactor = pow(
        max(dot(normal, halfwayDirection), 0.0),
        max(uMaterial.shininess, 1.0));

    vec3 diffuse = albedo * nDotL;
    vec3 specular = specularColor * specularFactor;
    return (diffuse + specular) * radiance;
}

vec3 evaluateDirectional(
    vec3 normal,
    vec3 viewDirection,
    vec3 albedo,
    vec3 specularColor)
{
    if (uDirectional.enabled == 0) {
        return vec3(0.0);
    }

    // direction = travel direction of light rays; L points toward the light.
    vec3 lightDirection = normalize(-uDirectional.direction);
    vec3 radiance = uDirectional.color * uDirectional.intensity;
    float shadow = 0.0;
    if (uDirectionalCastsShadow == 1) {
        shadow = sampleShadowMap2D(
            uDirectionalShadowMap,
            vLightSpacePosition,
            normal,
            lightDirection);
    }
    return (1.0 - shadow) * evaluateSurfaceLight(
        lightDirection,
        radiance,
        normal,
        viewDirection,
        albedo,
        specularColor);
}

vec3 evaluatePoint(
    int lightIndex,
    PointLight light,
    vec3 normal,
    vec3 viewDirection,
    vec3 albedo,
    vec3 specularColor)
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
    float shadow = samplePointShadow(lightIndex, light.position);
    return (1.0 - shadow) * evaluateSurfaceLight(
        lightDirection,
        radiance,
        normal,
        viewDirection,
        albedo,
        specularColor);
}

vec3 evaluateSpot(
    vec3 normal,
    vec3 viewDirection,
    vec3 albedo,
    vec3 specularColor)
{
    if (uSpot.enabled == 0) {
        return vec3(0.0);
    }

    vec3 toLight = uSpot.position - vWorldPosition;
    float distanceToLight = length(toLight);
    vec3 lightDirection = toLight / max(distanceToLight, 0.0001);

    // Spot direction = where the cone points. Compare against light→fragment.
    vec3 spotAxis = normalize(uSpot.direction);
    float cosTheta = dot(normalize(-lightDirection), spotAxis);

    // Cutoffs are cos(angle). Inner is narrower => larger cosine.
    float cosInner = uSpot.innerCutoff;
    float cosOuter = uSpot.outerCutoff;
    if (cosInner < cosOuter) {
        float tmp = cosInner;
        cosInner = cosOuter;
        cosOuter = tmp;
    }

    float cone = smoothstep(cosOuter, cosInner, cosTheta);
    float attenuation = 1.0 / max(
        uSpot.constantAttenuation +
        uSpot.linearAttenuation * distanceToLight +
        uSpot.quadraticAttenuation * distanceToLight * distanceToLight,
        0.0001);
    vec3 radiance = uSpot.color * uSpot.intensity * attenuation * cone;

    float shadow = 0.0;
    if (uSpotCastsShadow == 1) {
        shadow = sampleShadowMap2D(
            uSpotShadowMap,
            vSpotLightSpacePosition,
            normal,
            lightDirection);
    }

    return (1.0 - shadow) * evaluateSurfaceLight(
        lightDirection,
        radiance,
        normal,
        viewDirection,
        albedo,
        specularColor);
}

// Classic hemisphere ambient: sky on upward normals, ground on downward.
vec3 evaluateHemisphere(vec3 normal, vec3 albedo, float ao)
{
    if (uHemisphere.enabled == 0) {
        return vec3(0.0);
    }

    vec3 up = uHemisphere.upDirection;
    float upLength = length(up);
    if (upLength < 0.0001) {
        up = vec3(0.0, 1.0, 0.0);
    } else {
        up /= upLength;
    }

    float blend = clamp(dot(normal, up) * 0.5 + 0.5, 0.0, 1.0);
    vec3 ambient = mix(
        uHemisphere.groundColor,
        uHemisphere.skyColor,
        blend) * uHemisphere.intensity;

    return albedo * ambient * ao;
}

void main()
{
    vec3 albedo = sampleAlbedo();
    vec3 specularColor = sampleSpecular();
    vec3 emissive = sampleEmissive();
    float ao = sampleAo();
    vec3 normal = sampleNormal();
    vec3 viewDirection = normalize(uViewPosition - vWorldPosition);

    vec3 lighting = evaluateHemisphere(normal, albedo, ao);
    lighting += evaluateDirectional(normal, viewDirection, albedo, specularColor);

    int pointCount = min(uPointLightCount, MAX_POINT_LIGHTS);
    for (int index = 0; index < pointCount; ++index) {
        lighting += evaluatePoint(
            index,
            uPointLights[index],
            normal,
            viewDirection,
            albedo,
            specularColor);
    }

    lighting += evaluateSpot(normal, viewDirection, albedo, specularColor);
    lighting += emissive;

    vec3 mappedColor = lighting / (lighting + vec3(1.0));
    vec3 gammaCorrected = pow(mappedColor, vec3(1.0 / 2.2));
    fragmentColor = vec4(gammaCorrected, 1.0);
}
