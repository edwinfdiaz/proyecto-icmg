#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "libraries/stb_image.h"
#include "Origin.h"

// ============================================================
// CARGA DE TEXTURAS
// ============================================================

GLuint loadTexture(const char* filepath) {
	GLuint textureID; glGenTextures(1, &textureID); glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); int width; int height; int nrChannels;
	stbi_set_flip_vertically_on_load(true); unsigned char* data = stbi_load(filepath, &width, &height, &nrChannels, 0); if (data) {
		GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data); glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cerr << "Error al cargar la textura: " << filepath << std::endl;
	} stbi_image_free(data); return textureID;
}

// ============================================================
// SHADERS
// ============================================================

std::string readShaderFile(const std::string& filePath) {
	std::ifstream fileStream(filePath, std::ios::in); if (!fileStream.is_open()) {
		std::cerr << "ERROR: Failed to open shader file: " << filePath << std::endl; return "";
	} std::stringstream sstr;
	sstr << fileStream.rdbuf(); return sstr.str();
}

// Verificamos que el shader compile correctamente
static void printShaderLog(GLuint shader, const char* name) {
	GLint success = 0; glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar infoLog[1024]; glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog); std::cerr
			<< "ERROR: Shader compile failed (" << name << ")\n" << infoLog << std::endl;
	}
}

// Verificamos el linkeo del programa
static void printProgramLog(GLuint program) {
	GLint success = 0; glGetProgramiv(program, GL_LINK_STATUS, &success); if (!success) {
		GLchar infoLog[1024]; glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog); std::cerr << "ERROR: Program link failed\n"
			<< infoLog << std::endl;
	}
}

// Creamos el programa de shaders
GLuint createShaderProgram(const std::string& vertPath, const std::string& fragPath) {
	std::string vertCode = readShaderFile(vertPath);
	std::string fragCode = readShaderFile(fragPath); const char* vSource = vertCode.c_str(); const char* fSource = fragCode.c_str();

	// Vertex Shader
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER); glShaderSource(vShader, 1, &vSource, nullptr); glCompileShader(vShader);
	printShaderLog(vShader, "VERTEX");

	// Fragment Shader
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER); glShaderSource(fShader, 1, &fSource, nullptr); glCompileShader(fShader);
	printShaderLog(fShader, "FRAGMENT");

	// Programa
	GLuint program = glCreateProgram(); glAttachShader(program, vShader); glAttachShader(program, fShader); glLinkProgram(program);
	printProgramLog(program); glDeleteShader(vShader); glDeleteShader(fShader); return program;
}

// ============================================================
// ESFERAS
// ============================================================

struct VertexSphere { glm::vec3 pos; glm::vec4 color; glm::vec2 texCoords; };

// Representación de una esfera 3D
struct Sphere {
	GLuint VAO = 0; GLuint VBO = 0; GLuint EBO = 0; GLsizei indexCount = 0; float scale = 1.0f; glm::vec4 color =
		glm::vec4(1.0f); GLuint textureID = 0; void destroy() {
		if (VAO) glDeleteVertexArrays(1, &VAO); if (VBO) glDeleteBuffers(1, &VBO);
		if (EBO) glDeleteBuffers(1, &EBO);
	}
};

// ============================================================
// CREAR ESFERA
// ============================================================

void createSphere(Sphere& sphere, int stacks, int sectors) {
	std::vector<VertexSphere> vertices; std::vector<unsigned int> indices;

	// --------------------------------------------------------
	// CREAR VÉRTICES
	// --------------------------------------------------------

	for (int i = 0; i <= stacks; ++i) {
		float phi = glm::pi<float>() * static_cast<float>(i) / static_cast<float>(stacks);
		for (int j = 0; j <= sectors; ++j) {
			float theta = 2.0f * glm::pi<float>() * static_cast<float>(j) / static_cast<float>(sectors);
			VertexSphere vertex;

			// Posición del vértice
			vertex.pos[0] = std::sin(phi) * std::cos(theta); vertex.pos[1] = std::cos(phi); vertex.pos[2] = std::sin(phi) * std::sin(theta);

			// Color
			vertex.color[0] = sphere.color.r; vertex.color[1] = sphere.color.g; vertex.color[2] = sphere.color.b; vertex.color[3] = sphere.color.a;

			// Coordenadas de textura
			vertex.texCoords[0] = static_cast<float>(j) / static_cast<float>(sectors); vertex.texCoords[1] = static_cast<float>(i) / static_cast<float>(stacks);
			vertices.push_back(vertex);
		}
	}

	// --------------------------------------------------------
	// CREAR ÍNDICES
	// --------------------------------------------------------

	for (int i = 0; i < stacks; ++i) {
		for (int j = 0; j < sectors; ++j) {
			int first = i * (sectors + 1) + j; int second = first + sectors + 1;

			// Primer triángulo
			indices.push_back(static_cast<unsigned int>(first)); indices.push_back(static_cast<unsigned int>(second)); indices.push_back(static_cast<unsigned int>(first + 1));

			// Segundo triángulo
			indices.push_back(static_cast<unsigned int>(second)); indices.push_back(static_cast<unsigned int>(second + 1)); indices.push_back(static_cast<unsigned int>(first + 1));
		}
	}

	// --------------------------------------------------------
	// BUFFERS DE OPENGL
	// --------------------------------------------------------

	glGenVertexArrays(1, &sphere.VAO); glGenBuffers(1, &sphere.VBO); glGenBuffers(1, &sphere.EBO);
	glBindVertexArray(sphere.VAO);

	// VBO
	glBindBuffer(GL_ARRAY_BUFFER, sphere.VBO);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(VertexSphere)), vertices.data(), GL_STATIC_DRAW);

	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);

	// --------------------------------------------------------
	// ATRIBUTO 0: POSICIÓN
	// --------------------------------------------------------

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexSphere), (void*)offsetof(VertexSphere, pos));
	glEnableVertexAttribArray(0);

	// --------------------------------------------------------
	// ATRIBUTO 1: COLOR
	// --------------------------------------------------------

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexSphere), (void*)offsetof(VertexSphere, color));
	glEnableVertexAttribArray(1);

	// --------------------------------------------------------
	// ATRIBUTO 2: TEXTURA
	// --------------------------------------------------------

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexSphere), (void*)offsetof(VertexSphere, texCoords));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	sphere.indexCount = static_cast<GLsizei>(indices.size());
}

// ============================================================
// DIBUJAR ESFERA
// ============================================================

void drawSphere(const Sphere& sphere, GLint mvpLoc, const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model) {
	glm::mat4 mvp = projection * view * model;
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

	// Activamos textura
	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, sphere.textureID); glBindVertexArray(sphere.VAO);
	glDrawElements(GL_TRIANGLES, sphere.indexCount, GL_UNSIGNED_INT, 0); glBindVertexArray(0);
}

// ============================================================
// CÁMARA
// ============================================================

glm::vec3 cameraPos = glm::vec3(0.0f, 1.5f, 5.5f);
float cameraSpeed = 3.0f;

// Procesa el teclado
void processInput(GLFWwindow* window, float deltaTime) {
	float velocity = cameraSpeed * deltaTime;

	// W = adelante
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { cameraPos.z -= velocity; }

	// S = atrás
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { cameraPos.z += velocity; }

	// A = izquierda
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { cameraPos.x -= velocity; }

	// D = derecha
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { cameraPos.x += velocity; }

	// Q = subir
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) { cameraPos.y += velocity; }

	// E = bajar
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) { cameraPos.y -= velocity; }
}

// ============================================================
// MAIN
// ============================================================

int main() {

	// --------------------------------------------------------
	// INICIALIZAR GLFW
	// --------------------------------------------------------

	if (!glfwInit()) return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);

	GLFWwindow* window = glfwCreateWindow(1000, 700, "Sistema Solar 3D", nullptr, nullptr);

	if (!window) { glfwTerminate(); return -1; }

	glfwMakeContextCurrent(window);

	// --------------------------------------------------------
	// CARGAR GLAD
	// --------------------------------------------------------

	if (!gladLoadGL()) { glfwTerminate(); return -1; }

	glViewport(0, 0, 1000, 700);

	// --------------------------------------------------------
	// CONFIGURACIÓN OPENGL
	// --------------------------------------------------------

	glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS); glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// --------------------------------------------------------
	// SHADERS
	// --------------------------------------------------------

	GLuint shaderProgram = createShaderProgram("shaders/vertex.vert", "shaders/fragment.frag");
	GLint mvpLoc = glGetUniformLocation(shaderProgram, "uMVP");

	// --------------------------------------------------------
	// EJES
	// --------------------------------------------------------

	Origin origin;

	// --------------------------------------------------------
	// SOL
	// --------------------------------------------------------

	Sphere sun;
	sun.scale = 0.75f;
	sun.color = glm::vec4(1.0f, 0.65f, 0.05f, 0.45f);
	sun.textureID = loadTexture("texturas/sun.jpg");
	createSphere(sun, 32, 32);

	// --------------------------------------------------------
	// TIERRA
	// --------------------------------------------------------

	Sphere earth;
	earth.scale = 0.25f;
	earth.color = glm::vec4(0.05f, 0.35f, 1.0f, 1.0f);
	earth.textureID = loadTexture("texturas/earth.jpg");
	createSphere(earth, 24, 24);

	// --------------------------------------------------------
	// COLOR DE FONDO
	// --------------------------------------------------------

	glClearColor(0.01f, 0.01f, 0.03f, 1.0f);

	// --------------------------------------------------------
	// CONFIGURACIÓN DE TEXTURAS
	// --------------------------------------------------------

	glUseProgram(shaderProgram);
	GLint texLoc = glGetUniformLocation(shaderProgram, "uTexture");
	if (texLoc != -1) { glUniform1i(texLoc, 0); }

	// --------------------------------------------------------
	// BUCLE PRINCIPAL
	// --------------------------------------------------------

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgram);

		float currentFrame = static_cast<float>(glfwGetTime());
		static float lastFrame = static_cast<float>(glfwGetTime());
		float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		float t = currentFrame;

		// ----------------------------------------------------
		// TÍTULO CON POSICIÓN DE CÁMARA
		// ----------------------------------------------------

		char title[200];
		snprintf(title, sizeof(title), "Sistema Solar 3D | Camara X: %.2f | Y: %.2f | Z: %.2f", cameraPos.x, cameraPos.y, cameraPos.z);
		glfwSetWindowTitle(window, title);

		// ----------------------------------------------------
		// CÁMARA
		// ----------------------------------------------------

		processInput(window, deltaTime);
		int width; int height;
		glfwGetFramebufferSize(window, &width, &height);
		float aspect = (height > 0) ? static_cast<float>(width) / static_cast<float>(height) : 4.0f / 3.0f;

		// ----------------------------------------------------
		// PROYECCIÓN
		// ----------------------------------------------------

		glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 100.0f);

		// ----------------------------------------------------
		// VIEW / CÁMARA
		// ----------------------------------------------------

		glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		// ====================================================
		// SOL
		// ====================================================

		glm::mat4 sunModel = glm::scale(glm::mat4(1.0f), glm::vec3(sun.scale));
		drawSphere(sun, mvpLoc, projection, view, sunModel);

		// ====================================================
		// TIERRA
		// ====================================================

		glm::mat4 earthModel = glm::mat4(1.0f);

		// Órbita alrededor del Sol
		earthModel = glm::rotate(earthModel, t * 0.8f, glm::vec3(0.0f, 1.0f, 0.0f));

		// Distancia al Sol
		earthModel = glm::translate(earthModel, glm::vec3(2.0f, 0.0f, 0.0f));

		// Rotación propia
		earthModel = glm::rotate(earthModel, t * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));

		// Escala
		earthModel = glm::scale(earthModel, glm::vec3(earth.scale));

		drawSphere(earth, mvpLoc, projection, view, earthModel);

		// ====================================================
		// EJES
		// ====================================================

		origin.draw();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// --------------------------------------------------------
	// LIMPIEZA
	// --------------------------------------------------------

	glDeleteTextures(1, &sun.textureID);
	glDeleteTextures(1, &earth.textureID);
	sun.destroy();
	earth.destroy();
	glDeleteProgram(shaderProgram);
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}