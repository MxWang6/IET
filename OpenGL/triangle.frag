#version 330 core

in vec3 OurColor;
out vec4 color;

void main()
{
    color = vec4(OurColor, 1.0f);
}