// Do not touch this file
#version 430 core

// Uniforms definitions common to all vertex shaders
uniform mat4 sd_projectionMatrix;
uniform mat4 sd_modelMatrix;

// Vertex shader inputs/outputs
layout (location = 0) in vec4 vertexLocation;
out vec2 textureLocation;

void main(void)
{
    gl_Position = sd_projectionMatrix * sd_modelMatrix * vec4(vertexLocation.xy, 0.0, 1.0);
    textureLocation = vertexLocation.zw;
}
