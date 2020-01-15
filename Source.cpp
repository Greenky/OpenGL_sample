// GLEW треба подключати до GLFW.
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
#include <iostream>

// Вертекс шейдер
const GLchar* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
"}\0";

// Фрагментний шейдер
const GLchar* fragmentShaderSource =
"#version 330 core\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";

int main()
{
	//Ініціалізація GLFW
	glfwInit();
	//Настройка GLFW
	//Задається мінімальна версія OpenGL. 
	//Мажорна 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//Мінорна
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//Установка профайла для якого створюється контекст (кор профайл)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//Вимкнення можливості зміни розміру окна
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Створення вікна 800х600
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Робимо вікно активним
	glfwMakeContextCurrent(window);
	
	// Ініціалізація GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	// Встановлення поля відмальовки
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	//Triangle
	GLint success;
	GLchar infoLog[512];

	GLfloat vertices[] = {
	 0.0f,  0.5f, 0.0f,  // Верхній правий кут
	 0.5f, -0.5f, 0.0f,  // Нижній правий кут
	-0.5f, -0.5f, 0.0f,  // Нижній лівий кут
	-0.5f,  0.5f, 0.0f   // Верхній лівий кут
	};
	GLuint indices[] = {
	0, 1, 2,   // трикутник 1
	0, 2, 3    // трикутник 2
	};

	// VBO - vertex buffer objects - буфер що зберігає точки
	// VAO - Vertex Array Object - вершинний масив
	// EBO - Element Buffer Object - буфер індексів порядку відмальовки вершин
	GLuint VBO, VAO, EBO;
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	GLuint vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		std::fill(infoLog, infoLog + sizeof(infoLog), 0);
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR in Vertex Shader compilation: " << infoLog << std::endl;
	}

	GLuint fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		std::fill(infoLog, infoLog + sizeof(infoLog), 0);
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR in Fragment Shader compilation: " << infoLog << std::endl;
	}

	GLuint shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if(!success) {
		std::fill(infoLog, infoLog + sizeof(infoLog), 0);
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR in Shader Program compilation: " << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	//
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// Clear the colorbuffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}