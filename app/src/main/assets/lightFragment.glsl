#version 300 es

precision mediump float;
out vec4 FragColor;

uniform vec3 lightColor;

void main()
{
    FragColor = vec4(lightColor, 1.0); // set all 4 vector values to 1.0
}