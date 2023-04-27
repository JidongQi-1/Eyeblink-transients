//
// Copyright (c) 2018 Santini Designs. All rights reserved.
//

#version 430 core

// Uniform definitions
uniform sampler2D textureSampler;

// Fragment shader inputs/outputs
in vec2 textureLocation;
out vec4 fragmentColor;

void main(void)
{
    fragmentColor = texture(textureSampler, textureLocation);
}
