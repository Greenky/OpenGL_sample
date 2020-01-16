// GLEW треба подключати до GLFW.
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#define READ_SIZE 100

// Вертекс шейдер
const GLchar* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"out vec3 outColor;\n"
"void main()\n"
"{\n"
	"gl_Position = vec4(position, 1.0);\n"
	"outColor = color;\n"
"}\0";

// Фрагментний шейдер
const GLchar* fragmentShaderSource =
"#version 330 core\n"
"out vec4 color;\n"
"in vec3 outColor;\n"
"void main()\n"
"{\n"
	"color = vec4(outColor, 1.0f);\n"
"}\n\0";

char* get_file_content(FILE *fd)//(int fd)
{
	char* file_content = NULL;
	char* temp = NULL;

	int red = 0;
	int current = 0;
	char buffer[READ_SIZE];
	//while (red = read(fd, buffer, READ_SIZE))
	while (red = fread(buffer, sizeof(char), READ_SIZE, fd))
	{
		if (ferror(fd))// (red == -1)
			return NULL;
		else
		{
			if (file_content)
			{
				temp = (char*)malloc(current);
				//bzero(temp, current);
				std::fill(temp, temp + current, 0);
				for (int i = 0; i < current; i++)
					temp[i] = file_content[i];
				free(file_content);
			}
			file_content = (char*)malloc(current + READ_SIZE + 1);
			//bzero(file_content, current + READ_SIZE + 1);
			std::fill(file_content, file_content + current + READ_SIZE + 1, 0);
			if (temp)
			{
				for (int i = 0; i < current; i++)
					file_content[i] = temp[i];
				free(temp);
			}
			for (int i = 0; i < red; i++)
				file_content[current + i] = buffer[i];
			current += red;
			file_content[current] = 0;
			if (red < READ_SIZE)
				break;
		}
		if (feof(fd)) //(red == 0)
			break;
		std::fill(buffer, buffer + READ_SIZE, 0);
	}
	return file_content;
}

GLuint CreateShaderProgram_Files(const char* vertexShaderPath, const char* fragmentShaderPath)
{
	FILE *fdV, *fdF;
	fopen_s(&fdV, vertexShaderPath, "r");
	fopen_s(&fdF, fragmentShaderPath, "r");
	if (fdF == NULL || fdV == NULL)
	{
		std::cout << "Failed to open file(s)" << std::endl;
		return -1;
	}
	GLint success;
	GLchar infoLog[512];

	GLchar* fileString = (GLchar *)get_file_content(fdV);

	// Creating, and compiling vertex shader
	GLuint vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &fileString, NULL);
	glCompileShader(vertexShader);

	// Debug vertex shader code
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		std::fill(infoLog, infoLog + sizeof(infoLog), 0);
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR in Vertex Shader compilation: " << infoLog << std::endl;
		return -1;
	}
	free(fileString);


	fileString = (GLchar*)get_file_content(fdF);

	// Creating, and compiling fragment shader
	GLuint fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fileString, NULL);
	glCompileShader(fragmentShader);

	// Debug fragment shader code
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		std::fill(infoLog, infoLog + sizeof(infoLog), 0);
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR in Fragment Shader compilation: " << infoLog << std::endl;
		return -1;
	}
	free(fileString);

	// Creating shader program and attaching 2 shaders
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// Debug program compilation
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		std::fill(infoLog, infoLog + sizeof(infoLog), 0);
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR in Shader Program compilation: " << infoLog << std::endl;
		return -1;
	}
	// Deleting shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	//

	fclose(fdV);
	fclose(fdF);
	
	return shaderProgram;
}

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
	 0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // Верхній правий кут
	 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  // Нижній правий кут
	-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,  // Нижній лівий кут
	-0.5f,  0.5f, 0.0f, 0.3f, 0.3f, 0.3f   // Верхній лівий кут
	};
	GLuint indices[] = {
	0, 1, 2,   // трикутник 1
	0, 2, 3    // трикутник 2
	};

	// VBO - vertex buffer objects - буфер що зберігає точки
	// VAO - Vertex Array Object - вершинний масив
	// EBO - Element Buffer Object - буфер індексів порядку відмальовки вершин
	GLuint VBO, VAO, EBO;

	// creating VBO
	glGenBuffers(1, &VBO);
	// creating EBO
	glGenBuffers(1, &EBO);
	// creating VAO
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);


	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	// Vertex Shader draw mode
	// Vertexes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	//Colors
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	GLuint shaderProgram;
	shaderProgram = CreateShaderProgram_Files("vertexShader.shad", "fragmentShader.shad");

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Whireframe draw mode

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// Clear the colorbuffer - bgcolor
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		//Shader transformatin
		/*
		GLfloat timeValue = glfwGetTime();
		GLfloat greenValue = (sin(timeValue) / 2) + 0.5;
		GLint vertexColorLocation = glGetUniformLocation(shaderProgram, "outColor");
		*/
		glUseProgram(shaderProgram);
		/*
		glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
		*/
		//Draw image
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}
	// Delete all data
	glfwTerminate();
	return 0;
}