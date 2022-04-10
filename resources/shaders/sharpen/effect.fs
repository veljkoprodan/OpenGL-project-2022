#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D effectTexture;
uniform bool effect;

const float offset = 1.0 / 300.0;

void main() {

    if (!effect) {
        FragColor = texture(effectTexture, TexCoords);
        return;
    }

    vec2 offsets[9] = vec2[](
    vec2(-offset, offset),
    vec2(0.0, offset),
    vec2(offset, offset),
    vec2(-offset, 0.0),
    vec2(0.0, 0.0),
    vec2(0.0, offset),
    vec2(-offset, -offset),
    vec2(0.0, -offset),
    vec2(offset, -offset)
    );

    float kernel[9];

        kernel = float[](
        0, -1, 0,
        -1, 5, -1,
        0, -1, 0
        );



    vec3 sampleTex[9];
    for (int i = 0; i < 9; ++i) {
        sampleTex[i] = vec3(texture(effectTexture, TexCoords.st + offsets[i]));
    }

    vec3 color = vec3(0.0);
    for (int i = 0; i < 9; ++i) {
        color += sampleTex[i] * kernel[i];
    }

    FragColor = vec4(color, 1.0f);

    //    FragColor = vec4(average, average, average, 1.0f);
}