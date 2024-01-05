#version 430 core

in vec2 ioTexCoords;

out vec4 FragColor;

uniform sampler2D uTexture;

void main()
{
    vec3 texel = texture(uTexture, ioTexCoords).rgb;

    FragColor = vec4(texel, 1.0);
}
