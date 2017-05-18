#version 400

precision highp float;

layout (location = 0) in vec3 vp; // vector made from 3 floats

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

void main() {
    gl_Position = Projection * View * Model * vec4(vp, 1.0);
}
