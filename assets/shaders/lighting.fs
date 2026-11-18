#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform vec3 lightDir;
uniform float flashlight;
uniform float alarm;

out vec4 finalColor;

const float AMBIENT = 0.05;
const float CONE_INNER = 0.96;
const float CONE_OUTER = 0.84;
const float FLASHLIGHT_RANGE = 14.0;
const float FOG_START = 3.0;
const float FOG_END = 15.0;
const vec3 FOG_COLOR = vec3(0.01, 0.01, 0.015);
const vec3 ALARM_COLOR = vec3(0.55, 0.04, 0.03);

void main()
{
    vec4 albedo = texture(texture0, fragTexCoord) * colDiffuse;
    vec3 toFragment = fragPosition - viewPos;
    float distanceToEye = length(toFragment);
    vec3 direction = toFragment / distanceToEye;

    // Конус фонарика
    float cone = smoothstep(CONE_OUTER, CONE_INNER, dot(direction, normalize(lightDir)));
    float falloff = clamp(1.0 - distanceToEye / FLASHLIGHT_RANGE, 0.0, 1.0);
    float facing = max(dot(normalize(fragNormal), -direction), 0.15);
    float beam = flashlight * cone * falloff * falloff * facing * 2.2;

    vec3 light = vec3(AMBIENT) + vec3(1.0, 0.95, 0.8) * beam + ALARM_COLOR * alarm;
    vec3 color = albedo.rgb * light;

    float fog = clamp((distanceToEye - FOG_START) / (FOG_END - FOG_START), 0.0, 1.0);
    finalColor = vec4(mix(color, FOG_COLOR, fog), albedo.a);
}
