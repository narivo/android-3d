#version 300 es

precision mediump float;

out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform float ratio;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main() {
   FragColor = mix(texture(texture1, TexCoord), texture(texture2, vec2(-TexCoord.x, TexCoord.y)), ratio);
}