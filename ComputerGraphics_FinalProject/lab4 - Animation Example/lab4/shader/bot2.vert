#version 330 core

// Attributes
layout(location = 0) in vec3 inPosition;   // Vertex position
layout(location = 1) in vec3 inNormal;     // Vertex normal

// Uniforms
uniform mat4 MVP;                  // Model-View-Projection matrix

// Outputs to the fragment shader
out vec3 worldPosition;
out vec3 worldNormal;

void main() {
    // Directly use the input position and normal
    vec4 modelPosition = vec4(inPosition, 1.0);

    // Flip the normal along the Z-axis
    worldNormal = normalize(vec3(inNormal.x, inNormal.y, -inNormal.z));

    // Pass world-space data to the fragment shader
    worldPosition = vec3(modelPosition);

    // Transform to clip space
    gl_Position = MVP * modelPosition;
}
