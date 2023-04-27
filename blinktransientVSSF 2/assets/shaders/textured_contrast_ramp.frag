//
// Copyright (c) 2018 Santini Designs. All rights reserved.
//

#version 430 core

// Uniform definitions
uniform sampler2D textureSampler;
uniform float alpha;

// Fragment shader inputs/outputs
in vec2 textureLocation;
out vec4 fragmentColor;

void main(void)
{
    fragmentColor = vec4(texture(textureSampler, textureLocation).r,
                        texture(textureSampler, textureLocation).g,
                        texture(textureSampler, textureLocation).b,
                        alpha);
}
