// Include necessary headers
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

// GLM for matrix and vector math
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// TinyGLTF
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

// OpenGL headers
#include <glad/gl.h> // Use GLAD or GLEW depending on your setup
#include <GLFW/glfw3.h>

// Macros
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

// Global variables
GLFWwindow* window;
int windowWidth = 1024;
int windowHeight = 768;

// Camera parameters
// Move the camera back along the Z-axis
glm::vec3 eye_center(0.0f, 0.0f, 5.0f); // Camera position
glm::vec3 lookat(0.0f, 0.0f, 0.0f);     // Point to look at (model at origin)
glm::vec3 up(0.0f, 1.0f, 0.0f);         // Up vector remains the same
float FoV = 45.0f;
float zNear = 0.1f;
float zFar = 1500.0f;

// Camera parameters
glm::vec3 cameraPos(0.0f, 0.0f, 5.0f); // Camera position
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f); // Direction camera is looking
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f); // Up vector

// Timing
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f;

// Mouse control
float lastX = windowWidth / 2.0f;
float lastY = windowHeight / 2.0f;
float yaw = -90.0f; // Initialize yaw to -90 degrees to look towards negative Z
float pitch = 0.0f;
bool firstMouse = true;


// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// Helper functions to determine component sizes
size_t GetComponentSizeInBytes(int componentType);
size_t GetNumComponentsInType(int type);

// New animation-related structures
struct AnimationSampler {
    std::vector<float> inputs;  // Timestamps
    std::vector<glm::vec4> outputs; // Interpolation values (quaternions or vectors)
    std::string interpolation;
};

struct AnimationChannel {
    int samplerIndex;
    std::string targetPath; // "translation", "rotation", "scale"
    int nodeIndex;
};

struct Animation {
    std::vector<AnimationSampler> samplers;
    std::vector<AnimationChannel> channels;
    float duration;
};

// Global animation variables
std::vector<Animation> animations;
float animationTime = 0.0f;
bool isAnimationPlaying = true;

// Helper function to interpolate between keyframes
glm::vec4 interpolateKeyframes(
    const AnimationSampler& sampler,
    float animTime
) {
    // Find the appropriate keyframes
    auto it = std::lower_bound(
        sampler.inputs.begin(),
        sampler.inputs.end(),
        animTime
    );

    // If we're past the last keyframe, return the last value
    if (it == sampler.inputs.end()) {
        return sampler.outputs.back();
    }

    // Find indices of surrounding keyframes
    size_t nextIndex = std::distance(sampler.inputs.begin(), it);
    size_t prevIndex = (nextIndex > 0) ? nextIndex - 1 : 0;

    float prevTime = sampler.inputs[prevIndex];
    float nextTime = sampler.inputs[nextIndex];
    float t = (animTime - prevTime) / (nextTime - prevTime);

    // Linear interpolation
    if (sampler.interpolation == "LINEAR") {
        return glm::mix(
            sampler.outputs[prevIndex],
            sampler.outputs[nextIndex],
            t
        );
    }

    // For now, default to linear if interpolation is unknown
    return glm::mix(
        sampler.outputs[prevIndex],
        sampler.outputs[nextIndex],
        t
    );
}

// Update the main parsing of glTF to include animations
void parseAnimations(const tinygltf::Model& model) {
    for (const auto& anim : model.animations) {
        Animation animation;
        animation.duration = 0.0f;

        // Parse samplers
        for (const auto& sampler : anim.samplers) {
            AnimationSampler animSampler;

            // Input accessor (timestamps)
            const auto& inputAccessor = model.accessors[sampler.input];
            const auto& inputBufferView = model.bufferViews[inputAccessor.bufferView];
            const auto& inputBuffer = model.buffers[inputBufferView.buffer];

            size_t inputSize = GetComponentSizeInBytes(inputAccessor.componentType);
            size_t inputOffset = inputBufferView.byteOffset + inputAccessor.byteOffset;

            // Extract input timestamps
            for (size_t i = 0; i < inputAccessor.count; ++i) {
                float timestamp = *reinterpret_cast<const float*>(
                    &inputBuffer.data[inputOffset + i * inputSize]
                );
                animSampler.inputs.push_back(timestamp);
                animation.duration = std::max(animation.duration, timestamp);
            }

            // Output accessor (keyframe values)
            const auto& outputAccessor = model.accessors[sampler.output];
            const auto& outputBufferView = model.bufferViews[outputAccessor.bufferView];
            const auto& outputBuffer = model.buffers[outputBufferView.buffer];

            size_t outputSize = GetComponentSizeInBytes(outputAccessor.componentType);
            size_t outputOffset = outputBufferView.byteOffset + outputAccessor.byteOffset;
            size_t numComponents = GetNumComponentsInType(outputAccessor.type);

            // Extract output values (could be vec3 or vec4 depending on type)
            for (size_t i = 0; i < outputAccessor.count; ++i) {
                glm::vec4 value(0.0f);
                for (size_t j = 0; j < numComponents; ++j) {
                    value[j] = *reinterpret_cast<const float*>(
                        &outputBuffer.data[outputOffset + (i * numComponents + j) * outputSize]
                    );
                }
                animSampler.outputs.push_back(value);
            }

            animSampler.interpolation = sampler.interpolation.empty() ?
                "LINEAR" : sampler.interpolation;

            animation.samplers.push_back(animSampler);
        }

        // Parse channels
        for (const auto& channel : anim.channels) {
            AnimationChannel animChannel;
            animChannel.samplerIndex = channel.sampler;
            animChannel.targetPath = channel.target_path;
            animChannel.nodeIndex = channel.target_node;

            animation.channels.push_back(animChannel);
        }

        animations.push_back(animation);
    }
}

// Shader sources (updated with lighting)
const char* vertex_shader_source = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 FragPos; // Position of the fragment in world space
out vec3 Normal;  // Normal vector in world space

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Calculate the position of the fragment in world space
    FragPos = vec3(model * vec4(aPos, 1.0));
    // Transform the normal vector to world space
    Normal = mat3(transpose(inverse(model))) * aNormal;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;

in vec3 FragPos;    // From vertex shader
in vec3 Normal;     // From vertex shader

uniform vec3 lightPos;        // Position of the light source
uniform vec3 viewPos;         // Position of the camera
uniform vec4 baseColorFactor; // Base color from material
uniform vec3 emissiveFactor;  // Emissive color from material

void main()
{
    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * baseColorFactor.rgb;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * baseColorFactor.rgb;

    // Specular lighting
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float shininess = 32.0;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * vec3(1.0); // White specular light

    // Combine results and add emissive factor
    vec3 result = ambient + diffuse + specular + emissiveFactor;

    // Output final color with alpha from baseColorFactor
    FragColor = vec4(result, baseColorFactor.a);
}
)";

// Shader compilation and linking
GLuint LoadShader(const char* vertex_shader_source, const char* fragment_shader_source);

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        // Initially set to the mouse's starting position
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // Adjust this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Constrain the pitch to prevent screen flipping
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Update camera front vector
    glm::vec3 front;
    front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    front.y = sin(glm::radians(pitch));
    front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    cameraFront = glm::normalize(front);
}

int main()
{
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // OpenGL version (3.3 Core)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // For MacOS
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL glTF Loader", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    // Set framebuffer size callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load OpenGL functions (using GLAD)
    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL context\n";
        return -1;
    }

    // Background color
    glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

    // Enable depth test and face culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Load the glTF model using TinyGLTF
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err_str;
    std::string warn_str;

    // Load the model from a file (save your JSON content to 'model.gltf')
    bool ret = loader.LoadASCIIFromFile(&model, &err_str, &warn_str, "../lab4/model/car/scene.gltf");
    if (!warn_str.empty()) {
        std::cout << "Warn: " << warn_str << std::endl;
    }
    if (!err_str.empty()) {
        std::cerr << "Err: " << err_str << std::endl;
    }
    if (!ret) {
        std::cerr << "Failed to parse glTF\n";
        return -1;
    }


    // Prepare VAOs and VBOs
    struct Primitive {
        GLuint vao;
        GLuint indexCount;
        GLuint mode;
        GLuint indexType;
        int materialIndex; // Add this to store the material index
    };
    std::vector<Primitive> primitives;

    struct Material {
        glm::vec4 baseColorFactor;
        glm::vec3 emissiveFactor;
        std::string alphaMode;
        float alphaCutoff;
        bool doubleSided;
        // Add other material properties if needed (e.g., metallicFactor, roughnessFactor)
    };
    std::vector<Material> materials;

    // Prepare buffers for rendering
    for (const auto& mesh : model.meshes) {
        for (const auto& primitive : mesh.primitives) {
            // Create VAO
            GLuint vao;
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            // Prepare VBOs for attributes
            for (const auto& attrib : primitive.attributes) {
                const tinygltf::Accessor& accessor = model.accessors[attrib.second];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                GLuint vbo;
                glGenBuffers(1, &vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);

                size_t componentSize = GetComponentSizeInBytes(accessor.componentType);
                size_t numComponents = GetNumComponentsInType(accessor.type);
                size_t bufferSize = accessor.count * componentSize * numComponents;
                size_t dataOffset = bufferView.byteOffset + accessor.byteOffset;

                glBufferData(GL_ARRAY_BUFFER, bufferSize, &buffer.data[dataOffset], GL_STATIC_DRAW);

                int byteStride = accessor.ByteStride(bufferView);
                if (byteStride == 0) {
                    byteStride = numComponents * componentSize;
                }

                int loc = -1;
                if (attrib.first == "POSITION") {
                    loc = 0;
                } else if (attrib.first == "NORMAL") {
                    loc = 1;
                } else if (attrib.first == "TEXCOORD_0") {
                    loc = 2;
                }
                if (loc >= 0) {
                    glEnableVertexAttribArray(loc);
                    glVertexAttribPointer(loc, numComponents, accessor.componentType,
                        accessor.normalized ? GL_TRUE : GL_FALSE, byteStride, (void*)0);
                }
            }

            // Prepare EBO for indices
            if (primitive.indices >= 0) {
                const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& bufferView = model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                GLuint ebo;
                glGenBuffers(1, &ebo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

                size_t componentSize = GetComponentSizeInBytes(indexAccessor.componentType);
                size_t bufferSize = indexAccessor.count * componentSize;
                size_t dataOffset = bufferView.byteOffset + indexAccessor.byteOffset;

                glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, &buffer.data[dataOffset], GL_STATIC_DRAW);

                // Store the primitive
                Primitive prim;
                prim.vao = vao;
                prim.indexCount = indexAccessor.count;
                prim.mode = primitive.mode;
                prim.indexType = indexAccessor.componentType;
                // Set the material index
                prim.materialIndex = primitive.material; // This comes from the glTF primitive

                primitives.push_back(prim);
            }

            glBindVertexArray(0);
        }
    }

    for (const auto& mat : model.materials) {
        Material material;

        // Base Color Factor
        if (!mat.pbrMetallicRoughness.baseColorFactor.empty()) {
            material.baseColorFactor = glm::vec4(
                mat.pbrMetallicRoughness.baseColorFactor[0],
                mat.pbrMetallicRoughness.baseColorFactor[1],
                mat.pbrMetallicRoughness.baseColorFactor[2],
                mat.pbrMetallicRoughness.baseColorFactor[3]
            );
        } else {
            material.baseColorFactor = glm::vec4(1.0f); // Default to white if not specified
        }

        // Emissive Factor
        if (!mat.emissiveFactor.empty()) {
            material.emissiveFactor = glm::vec3(
                mat.emissiveFactor[0],
                mat.emissiveFactor[1],
                mat.emissiveFactor[2]
            );
        } else {
            material.emissiveFactor = glm::vec3(0.0f); // Default to black
        }

        // Alpha Mode
        material.alphaMode = mat.alphaMode.empty() ? "OPAQUE" : mat.alphaMode;

        // Alpha Cutoff (default to 0.5 if not specified)
        material.alphaCutoff = mat.alphaCutoff;

        // Double-Sided
        material.doubleSided = mat.doubleSided;

        materials.push_back(material);
    }

    // Set up shaders
    GLuint shaderProgram = LoadShader(vertex_shader_source, fragment_shader_source);

    // Set up transformation matrices
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::scale(model_matrix, glm::vec3(2.0f));
    float angle = glm::radians(270.0f); // Convert 90 degrees to radians
    glm::vec3 axis_z(1, 0, 0); // Z-axis
    model_matrix = glm::rotate(model_matrix, angle, axis_z);
    glm::mat4 view_matrix = glm::lookAt(eye_center, lookat, up);
    glm::mat4 projection_matrix = glm::perspective(glm::radians(FoV),
        (float)windowWidth / windowHeight, zNear, zFar);

    // Get uniform locations
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

    // After getting uniform locations for model, view, and projection matrices
    GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

    GLint baseColorFactorLoc = glGetUniformLocation(shaderProgram, "baseColorFactor");
    GLint emissiveFactorLoc = glGetUniformLocation(shaderProgram, "emissiveFactor");



    // Time tracking for FPS
    double lastTime = glfwGetTime();
    float deltaTime = 0.0f;
    float timeAccumulator = 0.0f;
    int frameCount = 0;
    // After loading the model, parse animations
    parseAnimations(model);
    // Render loop
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Input
        processInput(window);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        float cameraSpeed = 2.5f * deltaTime; // Adjust accordingly

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront; // Move forward
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront; // Move backward
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed; // Move left
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

        // FPS calculation
        frameCount++;
        timeAccumulator += deltaTime;
        if (timeAccumulator >= 2.0f) {
            double fps = frameCount / timeAccumulator;
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << "OpenGL glTF Loader | FPS: " << fps;
            glfwSetWindowTitle(window, ss.str().c_str());
            frameCount = 0;
            timeAccumulator = 0.0f;
        }

        if (isAnimationPlaying && !animations.empty()) {
            Animation& currentAnimation = animations[0];

            animationTime += deltaTime;
            if (animationTime > currentAnimation.duration) {
                animationTime = 0.0f; // Loop animation
            }

            model_matrix = glm::mat4(1.0f);
            /* Reset the model matrix before applying animations
            float angle = glm::radians(270.0f);
            glm::vec3 axis_z(1, 0, 0);
            model_matrix = glm::rotate(model_matrix, angle, axis_z);
            */
            // Update model transformations based on animation
            for (const auto& channel : currentAnimation.channels) {
                const auto& sampler = currentAnimation.samplers[channel.samplerIndex];
                glm::vec4 animValue = interpolateKeyframes(sampler, animationTime);

                if (channel.targetPath == "translation") {
                    // Apply translation
                    model_matrix = glm::translate(model_matrix, glm::vec3(
                        animValue.x, animValue.y, animValue.z
                    ));
                } else if (channel.targetPath == "rotation") {
                    // Apply rotation using quaternion
                    glm::quat rotation(animValue.w, animValue.x, animValue.y, animValue.z);
                    model_matrix *= glm::mat4_cast(rotation);
                } else if (channel.targetPath == "scale") {
                    // Apply scale
                    model_matrix = glm::scale(model_matrix, glm::vec3(
                        animValue.x, animValue.y, animValue.z
                    ));
                }
            }
        }

        glm::mat4 view_matrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Set light position (you can adjust these values)
        glm::vec3 lightPos(10.0f, 10.0f, 10.0f);
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

        // Set camera position
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

        // Render commands
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader program
        glUseProgram(shaderProgram);

        // Pass transformation matrices to the shader
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model_matrix));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view_matrix));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

        // Draw all primitives
        for (const auto& prim : primitives) {
            glBindVertexArray(prim.vao);

            // Retrieve the material for this primitive
            const Material& material = materials[prim.materialIndex];

            // Set material uniforms
            glUniform4fv(baseColorFactorLoc, 1, glm::value_ptr(material.baseColorFactor));
            glUniform3fv(emissiveFactorLoc, 1, glm::value_ptr(material.emissiveFactor));

            // Handle alpha mode
            if (material.alphaMode == "BLEND") {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            } else {
                glDisable(GL_BLEND);
            }

            // Handle double-sided materials
            if (material.doubleSided) {
                glDisable(GL_CULL_FACE);
            } else {
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
            }

            // Draw the primitive
            glDrawElements(prim.mode, prim.indexCount, prim.indexType, 0);
        }

        glBindVertexArray(0);

        // Extract and print the model's position
        glm::vec3 modelPosition = glm::vec3(model_matrix[3]); // Extract the translation part
        std::cout << "Model position: (" << modelPosition.x << ", " << modelPosition.y << ", " << modelPosition.z << ")" << std::endl;

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // Cleanup
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    // Close window on escape key
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Adjust viewport
    glViewport(0, 0, width, height);
}

// Shader loading utility function
GLuint LoadShader(const char* vertex_shader_source, const char* fragment_shader_source) {
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertexShader);
    // Check compile errors...
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Error: Vertex shader compilation failed\n" << infoLog << std::endl;
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragmentShader);
    // Check compile errors...
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Error: Fragment shader compilation failed\n" << infoLog << std::endl;
    }

    // Link shaders to create a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Check link errors...
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Error: Shader program linking failed\n" << infoLog << std::endl;
    }

    // Clean up shaders as they're linked into the program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Helper functions to determine component sizes
size_t GetComponentSizeInBytes(int componentType) {
    switch (componentType) {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        return 1;
    case TINYGLTF_COMPONENT_TYPE_SHORT:
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        return 2;
    case TINYGLTF_COMPONENT_TYPE_INT:
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
        return 4;
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
        return 8;
    default:
        return 0;
    }
}

size_t GetNumComponentsInType(int type) {
    switch (type) {
    case TINYGLTF_TYPE_SCALAR: return 1;
    case TINYGLTF_TYPE_VEC2: return 2;
    case TINYGLTF_TYPE_VEC3: return 3;
    case TINYGLTF_TYPE_VEC4: return 4;
    case TINYGLTF_TYPE_MAT2: return 4;  // 2x2 matrix
    case TINYGLTF_TYPE_MAT3: return 9;  // 3x3 matrix
    case TINYGLTF_TYPE_MAT4: return 16; // 4x4 matrix
    default: return 0;
    }
}
