#version 330 core

in vec3 vNormal;
in vec3 vWorldPos;

uniform vec4 uColor;

out vec4 FragColor;

void main() {
    vec3 lightDir = normalize(vec3(0.5f, 1.0f, 0.7f));
    float diffuse = max(dot(normalize(vNormal), -lightDir), 0.0f);
    vec3 litColor = uColor.rgb * (0.35f + diffuse * 0.85f);
    FragColor = vec4(litColor, uColor.a);
}
