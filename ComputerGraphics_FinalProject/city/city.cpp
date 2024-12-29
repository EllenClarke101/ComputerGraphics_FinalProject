#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <render/shader.h>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <vector>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static GLFWwindow *window;
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

static glm::vec3 eye;
static glm::vec3 lookat(0, 0, 0);
static glm::vec3 up(0, 1, 0);
static double X = 400, lastY = 300;
static float Y = -90.0f;
static float pitch = 0.0f;
static bool Mouse = true;
float lastFrame = 0.0f;
float deltaTime = 0.0f;
static float Azimuth = 0.f;
static float Polar = 0.f;
static float Distance = 600.0f;


// Function to load a texture from a file and set it up for OpenGL
static GLuint LoadTextureTileBox(const char *texture_file_path) {
    int w, h, channels; // Variables to store texture width, height, and number of channels
    stbi_set_flip_vertically_on_load(false); // Ensure the image is loaded without flipping vertically (default behavior)

    // Load the image file into memory
    uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3); // Load image with 3 color channels (RGB)
    
    GLuint texture; // Variable to hold the generated texture ID
    glGenTextures(1, &texture); // Generate an OpenGL texture object
    glBindTexture(GL_TEXTURE_2D, texture); // Bind the texture as a 2D texture

    // Set texture wrapping parameters for the S and T axes (horizontal and vertical)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Repeat texture horizontally
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Repeat texture vertically

    // Set filtering options for minification and magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Use mipmaps and linear filtering for minification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Use linear filtering for magnification

    // Check if the image was successfully loaded
    if (img) {
        // Specify a 2D texture image, with width, height, and pixel data from the loaded image
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);

        // Generate mipmaps for the texture to improve rendering at different distances
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        // Log an error message if the image failed to load
        std::cout << "Failed to load texture " << texture_file_path << std::endl;
    }

    // Free the memory used by the loaded image since it's no longer needed
    stbi_image_free(img);

    // Return the OpenGL texture ID
    return texture;
}

// Struct to represent a Skybox in the scene
struct Skybox {
    // Position and scale of the skybox
    glm::vec3 pos;
    glm::vec3 scale;

    // Vertex positions for the cube representing the skybox
    GLfloat vertex_buffer_data[72] = {

        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,

        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
    };

    // Color data for each vertex of the skybox
    GLfloat color_buffer_data[72] = {

        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,

        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,

        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
    };

    // Index buffer to define triangles for rendering the cube
    GLuint index_buffer_data[36] = {
        0, 3, 2,
        0, 2, 1,

        4, 7, 6,
        4, 6, 5,

        8, 11, 10,
        8, 10, 9,

        12, 15, 14,
        12, 14, 13,

        16, 19, 18,
        16, 18, 17,

        20, 23, 22,
        20, 22, 21,
    };

    // UV coordinates for texture mapping
    GLfloat uv_buffer_data[48] = {

        0.5f, 0.666f,
        0.25f, 0.666f,
        0.25f, 0.333f,
        0.5f, 0.333f,

        1.0f, 0.666f,
        0.75f, 0.666f,
        0.75f, 0.333f,
        1.0f, 0.333f,

        0.75f, 0.666f,
        0.5f, 0.666f,
        0.5f, 0.333f,
        0.75f, 0.333f,

        0.25f, 0.666f,
        0.0f, 0.666f,
        0.0f, 0.333f,
        0.25f, 0.333f,

        0.5f, 0.333f,
        0.25f, 0.333f,
        0.25f, 0.0f,
        0.5f, 0.0f,

        0.5f, 1.0f,
        0.25f, 1.0f,
        0.25f, 0.666f,
        0.5f, 0.666f,

    };

    // OpenGL buffers
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint colorBufferID;
    GLuint uvBufferID;
    GLuint textureID;

    // Shader variable IDs
    GLuint mvpMatrixID;
    GLuint textureSamplerID;
    GLuint programID;

    // Function to initialize the skybox
    void initialize(glm::vec3 pos, glm::vec3 scale) {

        this->pos = pos;
        this->scale = scale;
        
        // Generate and bind the Vertex Array Object (VAO)
        glGenVertexArrays(1, &vertexArrayID);
        glBindVertexArray(vertexArrayID);
        
        // Generate and upload vertex data
        glGenBuffers(1, &vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

        // Generate and upload color data
        glGenBuffers(1, &colorBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

        // Generate and upload UV data
        glGenBuffers(1, &uvBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

        // Generate and upload index data
        glGenBuffers(1, &indexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

        // Load shaders
        programID = LoadShadersFromFile("../city/skybox.vert", "../city/skybox.frag");
        if (programID == 0)
        {
            std::cerr << "Failed to load shaders." << std::endl;
        }

        // Load texture
        mvpMatrixID = glGetUniformLocation(programID, "MVP");
        textureID = LoadTextureTileBox("../city/sky.png");
        textureSamplerID =glGetUniformLocation(programID, "textureSampler");
    }

    // Function to render the skybox
    void render(glm::mat4 cameraMatrix) {
        glUseProgram(programID);
        
        // Bind and configure vertex attributes
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Bind index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

        // Create model matrix and calculate MVP
        glm::mat4 modelMatrix = glm::mat4();
        modelMatrix = glm::scale(modelMatrix, scale);
        glm::mat4 mvp = cameraMatrix * modelMatrix;
        glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

        // Bind texture
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(textureSamplerID, 0);



        // Draw the box
        glDrawElements(
            GL_TRIANGLES,
            36,
            GL_UNSIGNED_INT,
            (void*)0
        );

        // Disable vertex attributes
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }

    // Function to clean up allocated resources
    void cleanup() {
        glDeleteBuffers(1, &vertexBufferID);
        glDeleteBuffers(1, &colorBufferID);
        glDeleteBuffers(1, &indexBufferID);
        glDeleteVertexArrays(1, &vertexArrayID);
        glDeleteBuffers(1, &uvBufferID);
        glDeleteTextures(1, &textureID);
        glDeleteProgram(programID);
    }

};

// Struct to represent a Building in the scene
struct Building {
    // Position and scale of the building
    glm::vec3 pos;
    glm::vec3 scale;

    // Vertex data for the building (a cube shape)
    GLfloat vertex_buffer_data[72] = {
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,

        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
    };

    // Color data for each vertex of the skybox
    GLfloat color_buffer_data[72] = {
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,

        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,

        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
    };

    // Index data for drawing triangles (faces of the cube)
    GLuint index_buffer_data[36] = {
        0, 1, 2,
        0, 2, 3,

        4, 5, 6,
        4, 6, 7,

        8, 9, 10,
        8, 10, 11,

        12, 13, 14,
        12, 14, 15,

        16, 17, 18,
        16, 18, 19,

        20, 21, 22,
        20, 22, 23,
    };

    // UV mapping for texture coordinates
    GLfloat uv_buffer_data[48] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,

        0.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 0.0f,

        0.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 0.0f,
    };

    // OpenGL buffer IDs
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint colorBufferID;
    GLuint uvBufferID;
    GLuint textureID;

    // Shader and uniform variable IDs
    GLuint mvpMatrixID;
    GLuint textureSamplerID;
    GLuint programID;

    // Initialize the building with position, scale, and texture path
    void initialize(glm::vec3 pos, glm::vec3 scale, const char* texturePath="../city/building_texture.jpg") {
        this->pos = pos;
        this->scale = scale;

        // Scale the UV mapping to adapt to the building's size
        for (int i = 0; i < 24; ++i) {
            uv_buffer_data[2 * i + 0] *= scale.x / 50.0f; // Scale U based on building width
            uv_buffer_data[2 * i + 1] *= scale.y / 50.0f; // Scale V based on building height
        }

        // Generate and bind the Vertex Array Object
        glGenVertexArrays(1, &vertexArrayID);
        glBindVertexArray(vertexArrayID);

        // Generate and upload vertex data
        glGenBuffers(1, &vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

        // Generate and upload UV mapping data
        glGenBuffers(1, &uvBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

        // Generate and upload index data
        glGenBuffers(1, &indexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

        // Load shaders for rendering the building
        programID = LoadShadersFromFile("../city/box.vert", "../city/box.frag");
        // Get uniform variable IDs
        mvpMatrixID = glGetUniformLocation(programID, "MVP");
        textureSamplerID = glGetUniformLocation(programID, "textureSampler");
        // Load texture for the building
        textureID = LoadTextureTileBox(texturePath);
    }

    // Render the building
    void render(glm::mat4 cameraMatrix) {
      glUseProgram(programID);
      // Bind and configure vertex attributes
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

      // Bind index buffer for drawing
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

      // Create model matrix for position and scale
      glm::mat4 modelMatrix = glm::mat4(1.0f);
      modelMatrix = glm::translate(modelMatrix, pos);
      modelMatrix = glm::scale(modelMatrix, scale);

      // Calculate and send the MVP matrix to the shader
      glm::mat4 mvp = cameraMatrix * modelMatrix;
      glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

      // Bind and configure UV data
      glEnableVertexAttribArray(1);
      glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

      // Bind texture
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, textureID);
      glUniform1i(textureSamplerID, 0);

      // Draw the building as a set of triangles
      glDrawElements(
          GL_TRIANGLES,
          36,
          GL_UNSIGNED_INT,
          (void*)0
      );

      // Disable vertex attributes
      glDisableVertexAttribArray(0);
      glDisableVertexAttribArray(1);

    }

    // Cleanup allocated resources
    void cleanup() {
        glDeleteBuffers(1, &vertexBufferID);
        glDeleteBuffers(1, &colorBufferID);
        glDeleteBuffers(1, &indexBufferID);
        glDeleteVertexArrays(1, &vertexArrayID);
        glDeleteBuffers(1, &uvBufferID);
        glDeleteTextures(1, &textureID);
        glDeleteProgram(programID);
    }
};

// Struct to represent a road in the scene
struct Road {
    // OpenGL buffer and shader program IDs
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint uvBufferID;
    GLuint textureID;
    GLuint programID;
    GLuint mvpMatrixID;
    GLuint textureSamplerID;

    // Initialize the road
    void initialize() {
        // Vertex data representing a large rectangle for the road
        GLfloat vertex_buffer_data[] = {
            -1000.0f, -40.0f, -1000.0f,
            -1000.0f, -40.0f, 1000.0f,
            1000.0f, -40.0f, 1000.0f,
            1000.0f, -40.0f, -1000.0f
        };

        // UV texture coordinates for the road
        GLfloat uv_buffer_data[] = {
            0.0f, 0.0f,
            0.0f, 100.0f,
            100.0f, 100.0f,
            100.0f, 0.0f
        };

        // Generate and bind the Vertex Array Object
        glGenVertexArrays(1, &vertexArrayID);
        glBindVertexArray(vertexArrayID);

        // Generate and upload vertex data to the GPU
        glGenBuffers(1, &vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

        // Generate and upload UV mapping data to the GPU
        glGenBuffers(1, &uvBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

        // Load and compile the shaders for the road
        programID = LoadShadersFromFile("../city/road.vert", "../city/road.frag");
        
        // Get the uniform variable locations in the shader program
        mvpMatrixID = glGetUniformLocation(programID, "MVP");
        textureSamplerID = glGetUniformLocation(programID, "textureSampler");
        
        // Load the texture for the road surface
        textureID = LoadTextureTileBox("../city/road_texture.jpg");
    }

    // Render the road
    void render(glm::mat4 cameraMatrix) {
        // Use the shader program
        glUseProgram(programID);

        // Bind and configure the vertex data
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // Bind and configure the UV data
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

        // Create the Model matrix (identity, as the road is static)
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        
        // Calculate and send the MVP matrix to the shader
        glm::mat4 mvp = cameraMatrix * modelMatrix;
        glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

        // Bind the road texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glUniform1i(textureSamplerID, 0);

        // Draw the road as a triangle fan (connecting all vertices)
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // Disable the vertex attributes after drawing
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }

    // Cleanup resources to prevent memory leaks
    void cleanup() {
        glDeleteBuffers(1, &vertexBufferID);
        glDeleteBuffers(1, &uvBufferID);
        glDeleteVertexArrays(1, &vertexArrayID);
        glDeleteTextures(1, &textureID);
        glDeleteProgram(programID);
    }
};

int main(void) {
    // Initialize GLFW for window and OpenGL context management
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return -1;
    }

    // Set GLFW window hints to specify OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFW window with specified dimensions
    window = glfwCreateWindow(1024, 768, "Lab 2", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to open a GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Set input mode for cursor visibility and add callbacks for input
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(window, key_callback);

    // Load OpenGL functions using glad
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
    {
        std::cerr << "Failed to initialize OpenGL context." << std::endl;
        return -1;
    }

    // Enable depth testing and face culling for 3D rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Define containers for buildings
    std::vector<Building> buildings;
    std::vector<Building> buildings2;
    std::vector<Building> buildings3;

    // Initialize the skybox with position and scale
    Skybox skybox;
    skybox.initialize(glm::vec3(0, 0, 0), glm::vec3(1000, 1000, 1000));

    // Initialize the road
    Road road;
    road.initialize();
    
    // Procedurally generate buildings in a grid layout
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            Building b;
            // Randomly calculate building positions and dimensions
            float spacing = 200.0f;
            float randomOffsetX = static_cast<float>(rand()) / RAND_MAX * 50.0f;
            float randomOffsetZ = static_cast<float>(rand()) / RAND_MAX * 50.0f;
            float xPos = -800.0f + (col * spacing) + randomOffsetX;
            float zPos = -800.0f + (row * spacing) + randomOffsetZ;
            float height = 100.0f + static_cast<float>(rand()) / RAND_MAX * 200.0f;
            float buildingWidth = 30.0f + static_cast<float>(rand()) / RAND_MAX * 20.0f;
            float buildingDepth = 30.0f + static_cast<float>(rand()) / RAND_MAX * 20.0f;

            // Set the position and scale of the building
            glm::vec3 pos(xPos, height / 2, zPos);
            glm::vec3 scale(buildingWidth, height, buildingDepth);

            // Initialize and store the building
            b.initialize(pos, scale);
            buildings3.push_back(b);
        }
    }
    // Set the initial camera position using spherical coordinates
    eye.y = Distance * cos(Polar);
    eye.x = Distance * cos(Azimuth);
    eye.z = Distance * sin(Azimuth);

    // Define the projection matrix for the scene
    glm::mat4 viewMatrix, projectionMatrix;
    glm::float32 FoV = 90;
    glm::float32 zNear = 0.1f;
    glm::float32 zFar = 2000.0f;
    projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);

    // Main render loop
    do {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        viewMatrix = glm::lookAt(eye, lookat, up);
        glm::mat4 vp = projectionMatrix * viewMatrix;

        glDisable(GL_DEPTH_TEST);
        skybox.render(vp);
        glEnable(GL_DEPTH_TEST);

        road.render(vp);
        
        glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.0f);
        glm::mat4 viewMatrix = glm::lookAt(eye, lookat, up);
        glm::mat4 cameraMatrix = projectionMatrix * viewMatrix;

        for (auto &building : buildings) {
            building.render(vp);
        }
        for (auto &building : buildings2) {
            building.render(vp);
        }
        for (auto &building : buildings3) {
            building.render(vp);
        }
        
        glm::mat4 viewProjection = vp * viewMatrix;
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window));

    // Clean up allocated resources for all buildings
    for(auto &building : buildings) {
        building.cleanup();
    }

    for(auto &building : buildings2) {
        building.cleanup();
    }

    for(auto &building : buildings3) {
        building.cleanup();
    }
    // Terminate GLFW
    glfwTerminate();
    return 0;
}

// Callback function for keyboard input
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    float moveSpeed = 5.0f;
    glm::vec3 direction = glm::normalize(lookat - eye);
    glm::vec3 right = glm::normalize(glm::cross(direction, up));

    // Move camera forward
    if(key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        lookat += direction * moveSpeed;
        eye += direction * moveSpeed;
    }
    // Move camera backward
    if(key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        lookat -= direction * moveSpeed;
        eye -= direction * moveSpeed;
    }
    // Move camera left
    if(key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        lookat -= right * moveSpeed;
        eye -= right * moveSpeed;
    }
    // Move camera right
    if(key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
        lookat += right * moveSpeed;
        eye += right * moveSpeed;
    }
    // Close the window if Escape key is pressed
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

// Callback function for mouse input
static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (Mouse) {
        X = xpos;
        lastY = ypos;
        Mouse = false;
        return;
    }
    // Calculate mouse movement offsets
    float xoffset = float(xpos - X);
    float yoffset = float(lastY - ypos);
    X = xpos;
    lastY = ypos;

    // Adjust sensitivity for smooth camera rotation
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    Y += xoffset;
    pitch += yoffset;

    // Prevent camera from flipping
    if(pitch > 89.0f) {
        pitch = 89.0f;
    }
    if(pitch < -89.0f) {
        pitch = -89.0f;
    }

    // Calculate the new camera direction
    glm::vec3 direction;
    direction.x = cos(glm::radians(Y)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(Y)) * cos(glm::radians(pitch));
    lookat = eye + glm::normalize(direction);
}

