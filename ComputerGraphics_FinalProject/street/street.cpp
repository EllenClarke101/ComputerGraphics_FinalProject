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

static GLFWwindow *window;
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// OpenGL camera view parameters
static glm::vec3 eye_center;
static glm::vec3 lookat(0, 0, 0);
static glm::vec3 up(0, 1, 0);

// View control
static float viewAzimuth = 0.f;
static float viewPolar = 0.f;
static float viewDistance = 600.0f;

// Variables for cursor view control
static double lastX = 400, lastY = 300;
static float yaw = -90.0f;       //Facing negative z
static float pitch = 0.0f;
static bool firstMouse = true;


static GLuint LoadTextureTileBox(const char *texture_file_path) {
	int w, h, channels;
	stbi_set_flip_vertically_on_load(false);
	uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// To tile textures on a box, we set wrapping to repeat
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (img) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
		glGenerateMipmap(GL_TEXTURE_2D);
	} else {
		std::cout << "Failed to load texture " << texture_file_path << std::endl;
	}
	stbi_image_free(img);

	return texture;
}


struct Skybox {
	glm::vec3 position;		// Position of the box
	glm::vec3 scale;		// Size of the box in each axis

	GLfloat vertex_buffer_data[72] = {	// Vertex definition for a canonical box
		// Front face
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		// Back face
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,

		// Left face
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		// Right face
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,

		// Top face
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		// Bottom face
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
	};

	GLfloat color_buffer_data[72] = {
		// Front, red
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Back, yellow
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		// Left, green
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// Right, cyan
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		// Top, blue
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		// Bottom, magenta
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
	};

	GLuint index_buffer_data[36] = {		// 12 triangle faces of a box
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

    // TODO: Define UV buffer data
    // ---------------------------
    // ---------------------------

	GLfloat uv_buffer_data[48] = {
		// Front face (positive Z)
		0.5f, 0.666f,  // Top right
		0.25f, 0.666f, // Top left
		0.25f, 0.333f, // Bottom left
		0.5f, 0.333f,  // Bottom right

		// Back face (negative Z)
		1.0f, 0.666f,  // Top right
		0.75f, 0.666f, // Top left
		0.75f, 0.333f, // Bottom left
		1.0f, 0.333f,  // Bottom right


		0.75f, 0.666f, // Top left
		0.5f, 0.666f,  // Top right
		0.5f, 0.333f,  // Bottom right
		0.75f, 0.333f, // Bottom left

		// Left face (negative X) - Correct
		0.25f, 0.666f, // Top right
		0.0f, 0.666f,  // Top left
		0.0f, 0.333f,  // Bottom left
		0.25f, 0.333f, // Bottom right

		// Right face (positive X) - Correct

		0.5f, 0.333f,
		0.25f, 0.333f, // Top left
		  // Top right
		0.25f, 0.0f,
		0.5f, 0.0f,    // Bottom right
		  // Bottom left

		// Bottom face (negative Y) - Now using the previous Top UV coordinates
		0.5f, 1.0f,
		0.25f, 1.0f,   // Top left
		   // Top right
		0.25f, 0.666f,
		0.5f, 0.666f,  // Bottom right
		 // Bottom left

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

	void initialize(glm::vec3 position, glm::vec3 scale) {
		// Define scale of the building geometry
		this->position = position;
		this->scale = scale;

		// Create a vertex array object
		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		// Create a vertex buffer object to store the vertex data
		glGenBuffers(1, &vertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

		// Create a vertex buffer object to store the color data
        // TODO:
		glGenBuffers(1, &colorBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

		// TODO: Create a vertex buffer object to store the UV data
		// --------------------------------------------------------
        // --------------------------------------------------------
		glGenBuffers(1, &uvBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

		// Create an index buffer object to store the index data that defines triangle faces
		glGenBuffers(1, &indexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

		// Create and compile our GLSL program from the shaders
		programID = LoadShadersFromFile("../street/skybox.vert", "../street/skybox.frag");
		if (programID == 0)
		{
			std::cerr << "Failed to load shaders." << std::endl;
		}

		// Get a handle for our "MVP" uniform
		mvpMatrixID = glGetUniformLocation(programID, "MVP");

        // TODO: Load a texture
        textureID = LoadTextureTileBox("../street/sky.png");


        // TODO: Get a handle to texture sampler
        textureSamplerID =glGetUniformLocation(programID, "textureSampler");
	}

	void render(glm::mat4 cameraMatrix) {
		glUseProgram(programID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

		// TODO: Model transform
		// -----------------------
        glm::mat4 modelMatrix = glm::mat4();
        // Scale the box along each axis to make it look like a building
        modelMatrix = glm::scale(modelMatrix, scale);
        // -----------------------

		// Set model-view-projection matrix
		glm::mat4 mvp = cameraMatrix * modelMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

		// TODO: Enable UV buffer and texture sampler
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(textureSamplerID, 0);



		// Draw the box
		glDrawElements(
			GL_TRIANGLES,      // mode
			36,    			   // number of indices
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
	}

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





//Constructor for the 'box' that will act as a building
struct Building {
    glm::vec3 position;      //This is the position of the 'box'
    glm::vec3 scale;         // The size of the box for each axis

    GLfloat vertex_buffer_data[72] = {	// Vertex definition for a canonical box
		// Front face
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		// Back face
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,

		// Left face
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		// Right face
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,

		// Top face
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		// Bottom face
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
	};

	GLfloat color_buffer_data[72] = {
		// Front, red
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Back, yellow
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		// Left, green
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// Right, cyan
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		// Top, blue
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		// Bottom, magenta
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
	};

	GLuint index_buffer_data[36] = {		// 12 triangle faces of a box
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

    // TODO: Define UV buffer data
    // ---------------------------
    // ---------------------------
    GLfloat uv_buffer_data[48] = {
    	//Front
    	0.0f, 1.0f,
    	1.0f, 1.0f,
    	1.0f, 0.0f,
    	0.0f, 0.0f,

    	//Back
    	0.0f, 1.0f,
    	1.0f, 1.0f,
    	1.0f, 0.0f,
    	0.0f, 0.0f,

    	//Left
    	0.0f, 1.0f,
    	1.0f, 1.0f,
    	1.0f, 0.0f,
    	0.0f, 0.0f,

    	//Right
    	0.0f, 1.0f,
    	1.0f, 1.0f,
    	1.0f, 0.0f,
    	0.0f, 0.0f,

    	//Top - we do not want to texture the top
    	0.0f, 0.0f,
    	0.0f, 0.0f,
    	0.0f, 0.0f,
    	0.0f, 0.0f,

    	//Bottom - we do not want to texture the bottom
    	0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
    };

	//The OpenGL buffers
	GLuint vertexArrayID;
	GLuint vertexBufferID;
	GLuint indexBufferID;
	GLuint colorBufferID;
	GLuint uvBufferID;
	GLuint textureID;

	//Shader IDs
	GLuint mvpMatrixID;
	GLuint textureSamplerID;
	GLuint programID;


    //Function to initalise the buildings
    void initialize(glm::vec3 position, glm::vec3 scale, const char* texturePath="../street/building_texture.jpg") {
        // Define the scale of the building's geometry
        this->position = position;
        this->scale = scale;

        // Adjust UV mapping scale to avoid excessive repetition
        for (int i = 0; i < 24; ++i) {
            uv_buffer_data[2 * i + 0] *= scale.x / 50.0f; // Scale U based on building width
            uv_buffer_data[2 * i + 1] *= scale.y / 50.0f; // Scale V based on building height
        }

        // Create OpenGL buffers as usual
        glGenVertexArrays(1, &vertexArrayID);
        glBindVertexArray(vertexArrayID);

        glGenBuffers(1, &vertexBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

        glGenBuffers(1, &uvBufferID);
        glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

        glGenBuffers(1, &indexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

        programID = LoadShadersFromFile("../street/box.vert", "../street/box.frag");
        mvpMatrixID = glGetUniformLocation(programID, "MVP");
        textureID = LoadTextureTileBox(texturePath);
        textureSamplerID = glGetUniformLocation(programID, "textureSampler");
    }


    //Function to render in the objects
    void render(glm::mat4 cameraMatrix) {
      glUseProgram(programID);
      glEnableVertexAttribArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);


      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

      // Model transform

      glm::mat4 modelMatrix = glm::mat4(1.0f);
      // Scale the box along each axis to make it look like a building
      modelMatrix = glm::translate(modelMatrix, position);
      modelMatrix = glm::scale(modelMatrix, scale);


      // Set model-view-projection matrix
      glm::mat4 mvp = cameraMatrix * modelMatrix;
      glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

      //  Enable UV buffer and texture sampler

      glEnableVertexAttribArray(1);
      glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

      //Set textureSampler to use texture unit 0
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, textureID);
      glUniform1i(textureSamplerID, 0);


      // Draw the box
      glDrawElements(
	  	GL_TRIANGLES,      // mode
	  	36,    			   // number of indices
	  	GL_UNSIGNED_INT,   // type
	  	(void*)0           // element array buffer offset
	  );

      glDisableVertexAttribArray(0);
      glDisableVertexAttribArray(1);

    }

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



struct Floor {
	GLuint vertexArrayID;
	GLuint vertexBufferID;
	GLuint uvBufferID;
	GLuint textureID;
	GLuint programID;
	GLuint mvpMatrixID;
	GLuint textureSamplerID;

	void initialize() {
		//Vertex data for a large quad
		GLfloat vertex_buffer_data[] = {
			-1000.0f, -40.0f, -1000.0f,
			-1000.0f, -40.0f, 1000.0f,
			1000.0f, -40.0f, 1000.0f,
			1000.0f, -40.0f, -1000.0f
		};

		GLfloat uv_buffer_data[] = {
			0.0f, 0.0f,
			0.0f, 100.0f,
			100.0f, 100.0f,
			100.0f, 0.0f
		};

		glGenVertexArrays(1, &vertexArrayID);
		glBindVertexArray(vertexArrayID);

		glGenBuffers(1, &vertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

		glGenBuffers(1, &uvBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

		programID = LoadShadersFromFile("../street/floor.vert", "../street/floor.frag");
		mvpMatrixID = glGetUniformLocation(programID, "MVP");
		textureID = LoadTextureTileBox("../street/road_texture.jpg");
		textureSamplerID = glGetUniformLocation(programID, "textureSampler");
	}


	void render(glm::mat4 cameraMatrix) {
		glUseProgram(programID);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glm::mat4 modelMatrix = glm::mat4(1.0f);
		glm::mat4 mvp = cameraMatrix * modelMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(textureSamplerID, 0);

		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}


	void cleanup() {
		glDeleteBuffers(1, &vertexBufferID);
		glDeleteBuffers(1, &uvBufferID);
		glDeleteVertexArrays(1, &vertexArrayID);
		glDeleteTextures(1, &textureID);
		glDeleteProgram(programID);
	}
};

struct Blade {
    GLuint VAO, VBO;
    GLuint texture;
};

struct WindTurbine {
    glm::vec3 position;
    glm::vec3 scale;
    GLuint baseVAO, baseVBO, baseTexture;
    Blade blade;
    float rotationAngle;
    GLuint shaderProgram;
    GLint mvpLocation;
};

void initializeWindTurbine(WindTurbine& turbine, const char* baseTexturePath, const char* bladeTexturePath) {
    // Base vertices (simplified cylinder)
    std::vector<float> baseVertices;
    for (int i = 0; i <= 360; i += 10) {
        float angle = glm::radians(float(i));
        baseVertices.push_back(0.5f * glm::cos(angle));
        baseVertices.push_back(0.0f);
        baseVertices.push_back(0.5f * glm::sin(angle));
    }

    glGenVertexArrays(1, &turbine.baseVAO);
    glGenBuffers(1, &turbine.baseVBO);
    glBindVertexArray(turbine.baseVAO);
    glBindBuffer(GL_ARRAY_BUFFER, turbine.baseVBO);
    glBufferData(GL_ARRAY_BUFFER, baseVertices.size() * sizeof(float), baseVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Blade vertices (simplified rectangle)
    float bladeVertices[] = {
        -0.05f, 0.0f, 0.0f,
         0.05f, 0.0f, 0.0f,
         0.05f, 2.0f, 0.0f,
        -0.05f, 2.0f, 0.0f
    };

    glGenVertexArrays(1, &turbine.blade.VAO);
    glGenBuffers(1, &turbine.blade.VBO);
    glBindVertexArray(turbine.blade.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, turbine.blade.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bladeVertices), bladeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Load textures
    turbine.baseTexture = LoadTextureTileBox(baseTexturePath);
    turbine.blade.texture = LoadTextureTileBox(bladeTexturePath);

    // Load shaders
    turbine.shaderProgram = LoadShadersFromFile("../street/turbine.vert", "../street/turbine.frag");
    turbine.mvpLocation = glGetUniformLocation(turbine.shaderProgram, "MVP");

    turbine.rotationAngle = 0.0f;
}

void renderWindTurbine(WindTurbine& turbine, const glm::mat4& viewProjection) {
    glUseProgram(turbine.shaderProgram);

    // Render base
    glm::mat4 baseModel = glm::translate(glm::mat4(1.0f), turbine.position);
    baseModel = glm::scale(baseModel, glm::vec3(turbine.scale.x, turbine.scale.y, turbine.scale.x));
    glm::mat4 baseMVP = viewProjection * baseModel;
    glUniformMatrix4fv(turbine.mvpLocation, 1, GL_FALSE, glm::value_ptr(baseMVP));

    glBindVertexArray(turbine.baseVAO);
    glBindTexture(GL_TEXTURE_2D, turbine.baseTexture);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 37);

    // Render blades
    glm::mat4 bladeModel = glm::translate(glm::mat4(1.0f), turbine.position + glm::vec3(0, turbine.scale.y, 0));
    bladeModel = glm::rotate(bladeModel, turbine.rotationAngle, glm::vec3(0, 0, 1));
    bladeModel = glm::scale(bladeModel, glm::vec3(turbine.scale.z));

    for (int i = 0; i < 3; ++i) {
        glm::mat4 rotatedBladeModel = glm::rotate(bladeModel, glm::radians(120.0f * i), glm::vec3(1, 0, 0));
        glm::mat4 bladeMVP = viewProjection * rotatedBladeModel;
        glUniformMatrix4fv(turbine.mvpLocation, 1, GL_FALSE, glm::value_ptr(bladeMVP));

        glBindVertexArray(turbine.blade.VAO);
        glBindTexture(GL_TEXTURE_2D, turbine.blade.texture);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    // Update rotation angle
    turbine.rotationAngle += 0.01f;
}


int main(void) {
  // Initialise GLFW
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW." << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Lab 2", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Failed to open a GLFW window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);   //Hide and capture cursor
	glfwSetCursorPosCallback(window, mouse_callback);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetKeyCallback(window, key_callback);

	// Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
	int version = gladLoadGL(glfwGetProcAddress);
	if (version == 0)
	{
		std::cerr << "Failed to initialize OpenGL context." << std::endl;
		return -1;
	}

	// Background
	//glClearColor(0.2f, 0.2f, 0.25f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	std::vector<Building> buildings;
	std::vector<Building> buildings2;
	std::vector<Building> buildings3;

	Skybox skybox;
	skybox.initialize(glm::vec3(0, 0, 0), glm::vec3(1000, 1000, 1000));

	Floor floor;
	floor.initialize();
    
    WindTurbine turbine;
    turbine.position = glm::vec3(0.0f, -40.0f, -100.0f);
    turbine.scale = glm::vec3(10.0f, 50.0f, 30.0f);
    initializeWindTurbine(turbine, "../street/turbine_base_texture.jpg", "../street/turbine_blade_texture.jpg");

	//Create multiple buildings
    for (int i = 0; i < 8; ++i) {
        Building b;

        // Set random x and z positions, ensuring enough space between buildings
        float xPos = i * 100.0f; // Spacing along x-axis
        float zPos = i;          // Spacing along z-axis

        glm::vec3 position(xPos, 110, zPos);

        // Set random height for each building within a range
        float height = 150;             // Height of the building
        glm::vec3 scale(30, height, 30); // Width, height, depth

        b.initialize(position, scale); // Pass position and scale
        buildings.push_back(b);
    }

	for (int i=0; i< 8; ++i) {
		Building b;

		float xPos = i * 100.0f ;
		float zPos = i + 200.0f;
		glm::vec3 position(xPos, 110, zPos);

		float height = 150 ;
		glm::vec3 scale(30, height, 30);
		b.initialize(position, scale);
		buildings2.push_back(b);
	}
    // ---------------------------

	// Create buildings in a grid layout with better spacing
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 4; ++col) {
            Building b;

            // Fixed minimum spacing between buildings (200 units)
            float spacing = 200.0f;

            // Random offset for each building (up to 50 units)
            float randomOffsetX = static_cast<float>(rand()) / RAND_MAX * 50.0f;
            float randomOffsetZ = static_cast<float>(rand()) / RAND_MAX * 50.0f;

            // Calculate position with spacing and offset
            float xPos = -800.0f + (col * spacing) + randomOffsetX;
            float zPos = -800.0f + (row * spacing) + randomOffsetZ;

            // Random height between 100 and 300 units
            float height = 100.0f + static_cast<float>(rand()) / RAND_MAX * 200.0f;

            // Random building size between 30 and 50 units
            float buildingWidth = 30.0f + static_cast<float>(rand()) / RAND_MAX * 20.0f;
            float buildingDepth = 30.0f + static_cast<float>(rand()) / RAND_MAX * 20.0f;

            glm::vec3 position(xPos, height / 2, zPos);
            glm::vec3 scale(buildingWidth, height, buildingDepth);

            b.initialize(position, scale); // Pass position and scale
            buildings3.push_back(b);
        }
    }


	// Camera setup
    eye_center.y = viewDistance * cos(viewPolar);
    eye_center.x = viewDistance * cos(viewAzimuth);
    eye_center.z = viewDistance * sin(viewAzimuth);

	glm::mat4 viewMatrix, projectionMatrix;
    glm::float32 FoV = 90;
	glm::float32 zNear = 0.1f;
	glm::float32 zFar = 2000.0f;
	projectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, zNear, zFar);

    do {
        float deltaTime = glfwGetTime(); // Use time since last frame for smooth rotation
        glfwSetTime(0.0);               // Reset time counter

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        viewMatrix = glm::lookAt(eye_center, lookat, up);
        glm::mat4 vp = projectionMatrix * viewMatrix;

        glDisable(GL_DEPTH_TEST);
        skybox.render(vp);
        glEnable(GL_DEPTH_TEST);

        floor.render(vp);
        
        // Render buildings
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
        renderWindTurbine(turbine, viewProjection);
        
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    } while (!glfwWindowShouldClose(window));

	for(auto &building : buildings) {
		building.cleanup();
	}

	for(auto &building : buildings2) {
		building.cleanup();
	}

	for(auto &building : buildings3) {
		building.cleanup();
	}

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}


// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	float moveSpeed = 5.0f;
	glm::vec3 direction = glm::normalize(lookat - eye_center);
	glm::vec3 right = glm::normalize(glm::cross(direction, up));

	if(key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		lookat += direction * moveSpeed;
		eye_center += direction * moveSpeed;
	}
	if(key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		lookat -= direction * moveSpeed;
		eye_center -= direction * moveSpeed;
	}
	if(key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		lookat -= right * moveSpeed;
		eye_center -= right * moveSpeed;
	}
	if(key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		lookat += right * moveSpeed;
		eye_center += right * moveSpeed;
	}

	//Close the window
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}


static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
		return;
	}

	float xoffset = float(xpos - lastX);
	float yoffset = float(lastY - ypos);
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	//Prevent flipping
	if(pitch > 89.0f) {
		pitch = 89.0f;
	}
	if(pitch < -89.0f) {
		pitch = -89.0f;
	}

	//Calculate new direction
	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	lookat = eye_center + glm::normalize(direction);
}
