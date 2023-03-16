#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 readyColor;


void main(){
    readyColor = color;
    gl_Position = vec4(position, 1.0f);
}