#version 330 core

in vec3 vNormal;
in vec3 vWorldPos;

uniform vec4 uColor;
uniform vec3 uAmbient;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uCameraPos;

out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uLightDir);
    vec3 V = normalize(uCameraPos - vWorldPos);
    vec3 H = normalize(L + V);

    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 32.0);
    vec3 lit = uColor.rgb * (uAmbient + uLightColor * diff + vec3(0.5) * spec);
    FragColor = vec4(lit, uColor.a);
}
