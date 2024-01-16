#version 430 core

in vec2 io_tex_coords;

out vec4 frag_color;

uniform sampler2D u_texture;

void main()
{
    vec3 texel = texture(u_texture, io_tex_coords).rgb;

    frag_color = vec4(texel, 1.0);
}
