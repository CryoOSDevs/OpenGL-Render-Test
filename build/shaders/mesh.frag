#version 330 core

in vec3 vNormal;
in vec3 vWorldPos;

uniform vec4 uColor;
uniform vec3 uAmbient;
uniform vec3 uDirLightDir;
uniform vec3 uDirLightColor;
uniform int uLightCount;
uniform vec3 uPointLightPos[8];
uniform vec3 uPointLightColor[8];
uniform vec3 uCameraPos;
uniform float uSelected;

uniform int uUseGrid;
uniform float uGridScale;

out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vWorldPos);

    vec3 direct = vec3(0.0);
    vec3 Ld = normalize(-uDirLightDir);
    float diffDir = max(dot(N, Ld), 0.0);
    vec3 Hd = normalize(Ld + V);
    float specDir = pow(max(dot(N, Hd), 0.0), 32.0);
    direct += uDirLightColor * diffDir;
    direct += uDirLightColor * specDir * 0.35;

    for (int i = 0; i < 8; ++i) {
        if (i >= uLightCount) {
            break;
        }
        vec3 pointVec = uPointLightPos[i] - vWorldPos;
        float pointDist = length(pointVec);
        vec3 Lp = normalize(pointVec);
        float pointAtten = 1.0 / (1.0 + 0.18 * pointDist + 0.032 * pointDist * pointDist);
        float diffPoint = max(dot(N, Lp), 0.0);
        vec3 Hp = normalize(Lp + V);
        float specPoint = pow(max(dot(N, Hp), 0.0), 32.0);
        direct += uPointLightColor[i] * diffPoint * pointAtten;
        direct += uPointLightColor[i] * specPoint * pointAtten * 0.45;
    }

    vec3 lit = uColor.rgb * (uAmbient + direct);

    // optional grid overlay for ground
    if (uUseGrid == 1) {
        float gridScale = uGridScale;
        vec2 coord = vWorldPos.xz * gridScale;
        float line = (abs(fract(coord.x - 0.5) - 0.5) + abs(fract(coord.y - 0.5) - 0.5));
        float grid = smoothstep(0.48, 0.5, line);
        vec3 gridColor = mix(vec3(0.12), vec3(0.2), grid);
        lit = mix(lit, gridColor, 0.45);
    }

    if (uSelected > 0.5) {
        lit = mix(lit, vec3(0.75, 0.9, 1.0), 0.55);
    }

    FragColor = vec4(lit, uColor.a);
}
