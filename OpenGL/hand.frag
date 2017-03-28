#version 330 core

in vec2 TexCoords;
in vec3 OurColor;

out vec4 color;

uniform sampler2D texture_diffuse1;

void main()
{
    color = vec4(texture(texture_diffuse1, TexCoords));
    
   //color = vec4(0.9f, 0.7f, 1.5f, 0.0f);
}