#include <iostream>

extern "C" {
#include <glew.h>
// GLFW
#include <glfw3.h>
}


// Function prototypes
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Window dimensions
const GLuint WIDTH = 384, HEIGHT = 216;


const int pixel_w = 384, pixel_h = 216;
unsigned char buffer[pixel_w * pixel_h * 3];


// Shaders
const GLchar *VERTEX_SHADER =
        "#version 330 core\n"
        "layout (location = 0) in vec3 position;\n"
        "layout (location = 1) in vec3 color;\n"
        "layout (location = 2) in vec2 texCoord;\n"
        "out vec3 ourColor;\n"
        "out vec2 TexCoord;\n"
        "void main(){\n"
        "    gl_Position = vec4(position, 1.0f);\n"
        "    ourColor = color;\n"
        "    TexCoord = texCoord;\n"
        "}\n";

const GLchar *FRAGMENT_SHADER =
        "#version 330 core\n"
        "in vec3 ourColor;\n"
        "in vec2 TexCoord;\n"
        "out vec4 color;\n"
        "uniform sampler2D ourTexture;\n"
        "void main(){\n"
        "    color = texture(ourTexture, TexCoord);\n"
        "}\n";

// The MAIN function, from here we start the application and run the game loop
int main() {
    const char *src_filename = "a-frame-384x216.rgb24";
    FILE *fp = fopen(src_filename, "rb+");
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);   // 主版本
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);   // 副版本(可选)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);    // 设置苹果系统向后兼容

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();

    // Define the viewport dimensions
    glViewport(0, 0, pixel_w, pixel_h);


    // Build and compile our shader program
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &VERTEX_SHADER, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &FRAGMENT_SHADER, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up vertex data (and buffer(s)) and attribute pointers
    GLfloat vertices[] = {
            //     ---- 位置 ----       ---- 颜色 ----     - 纹理坐标 -
            1.0f,  1.0f,  0.0f,    1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // 右上
            1.0f, -1.0f,  0.0f,    0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // 右下
            -1.0f, -1.0f, 0.0f,    0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 左下
            -1.0f, 1.0f,  0.0f,    1.0f, 1.0f, 0.0f,   0.0f, 0.0f  // 左上
    };
    //从0开始，画两个三角形，组成矩形（从0开始，画两个三角形的索引即可）
    GLuint indices[] = {  // Note that we start from 0!
            0, 1, 3, // First Triangle
            1, 2, 3  // Second Triangle
    };
    GLuint VBO, VAO, EBO;
    // 1. 绑定顶点数组对象
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *) 0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // TexCoord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *) (6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0); // Unbind VAO


    // Load and create a texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D,
                  texture); // All upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // Set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_REPEAT);    // Set texture wrapping to GL_REPEAT (usually basic wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Load image, create texture and generate mipmaps


    fread(buffer, 1, pixel_w * pixel_h * 3, fp);

    // Game loop
    while (/*!feof(fp) && */!glfwWindowShouldClose(window)) {

        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Bind Texture
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pixel_w, pixel_h, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);

        // Activate shader
        glUseProgram(shaderProgram);

        // Draw container
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();

    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}