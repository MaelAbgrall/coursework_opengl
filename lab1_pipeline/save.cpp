// 03_Light


#pragma comment(linker, "/NODEFAULTLIB:MSVCRT")

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
using namespace std;


#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <assimp/Importer.hpp>

void errorCallbackGLFW(int error, const char* description);
void hintsGLFW();
void endProgram();
void render(GLfloat currentTime);
void update(GLfloat currentTime);
void setupRender();
void startup();
void onResizeCallback(GLFWwindow* window, int w, int h);
void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void onMouseMoveCallback(GLFWwindow* window, double x, double y);
void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);
void debugGL();
static void APIENTRY openGLDebugCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const GLvoid* userParam);
string readShader(string name);
void checkErrorShader(GLuint shader);
void readObj(string name, struct modelObject *obj);
void readTexture(string name, GLuint texture);


// VARIABLES
GLFWwindow*		window;
int				windowWidth = 640;
int				windowHeight = 480;
bool			running = true;
glm::mat4		proj_matrix;
glm::vec3		modelAngle = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3		modelDisp = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3		cameraPosition = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3		cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float           aspect = (float)windowWidth / (float)windowHeight;
float			fovy = 45.0f;
bool			keyStatus[1024];
GLfloat			deltaTime = 0.0f;
GLfloat			lastTime = 0.0f;

// FPS camera variables
GLfloat			yaw = -90.0f;	// init pointing to inside
GLfloat			pitch = 0.0f;	// start centered
GLfloat			lastX = (GLfloat)windowWidth / 2.0f;	// start middle screen
GLfloat			lastY = (GLfloat)windowHeight / 2.0f;	// start middle screen
bool			firstMouse = true;

//post processing
//unsigned int framebuffer;
//unsigned int textureColorbuffer;
//GLuint FBprogram;
//unsigned int quadVAO;
float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
};


// OBJ Variables
struct modelObject {
    std::vector < glm::vec3 > out_vertices;
    std::vector < glm::vec2 > out_uvs;
    std::vector < glm::vec3 > out_normals;
    GLuint*		texture;
    GLuint      program;
    GLuint      vao;
    GLuint      buffer[2];
    GLint       mv_location;
    GLint       proj_location;
    GLint		tex_location;

    // extra variables for this example
    GLuint		matColor_location;
    GLuint		lightColor_location;

} rockStruct, groundStruct, towerStruct, fanStruct, fbStruct, lightModel;

glm::vec3		*rockPositions;
glm::vec3		*rockRotations;

// Light
bool			movingLight = true;
glm::vec3		lightDisp = glm::vec3(-0.09f, 0.6f, -1.2f);
glm::vec3		ia = glm::vec3(0.5f, 0.5f, 0.5f);
GLfloat			ka = 1.0f;
glm::vec3		id = glm::vec3(0.93f, 0.75f, 0.32f);
GLfloat			kd = 1.0f;
glm::vec3		is = glm::vec3(1.00f, 1.00f, 1.0f);
GLfloat			ks = 0.01f;

#pragma region main, debug, setup and end
int main()
{

    if (!glfwInit()) {							// Checking for GLFW
        cout << "Could not initialise GLFW...";
        return 0;
    }

    glfwSetErrorCallback(errorCallbackGLFW);	// Setup a function to catch and display all GLFW errors.

    hintsGLFW();								// Setup glfw with various hints.		

                                                // Start a window using GLFW
    string title = "Coursework demo";

    // Fullscreen
    //const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    //windowWidth = mode->width; windowHeight = mode->height;
    //window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), glfwGetPrimaryMonitor(), NULL);

    // Window
    window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), NULL, NULL);
    if (!window) {								// Window or OpenGL context creation failed
        cout << "Could not initialise GLFW...";
        endProgram();
        return 0;
    }

    glfwMakeContextCurrent(window);				// making the OpenGL context current

                                                // Start GLEW (note: always initialise GLEW after creating your window context.)
    glewExperimental = GL_TRUE;					// hack: catching them all - forcing newest debug callback (glDebugMessageCallback)
    GLenum errGLEW = glewInit();
    if (GLEW_OK != errGLEW) {					// Problems starting GLEW?
        cout << "Could not initialise GLEW...";
        endProgram();
        return 0;
    }

    debugGL();									// Setup callback to catch openGL errors.	

                                                // Setup all the message loop callbacks.
    glfwSetWindowSizeCallback(window, onResizeCallback);		// Set callback for resize
    glfwSetKeyCallback(window, onKeyCallback);					// Set Callback for keys
    glfwSetMouseButtonCallback(window, onMouseButtonCallback);	// Set callback for mouse click
    glfwSetCursorPosCallback(window, onMouseMoveCallback);		// Set callback for mouse move
    glfwSetScrollCallback(window, onMouseWheelCallback);		// Set callback for mouse wheel.
                                                                //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);	// Set mouse cursor. Fullscreen
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	// Set mouse cursor FPS.

    setupRender();								// setup some render variables.
    startup();									// Setup all necessary information for startup (aka. load texture, shaders, models, etc).

    do {										// run until the window is closed
        GLfloat currentTime = (GLfloat)glfwGetTime();		// retrieve timelapse
        deltaTime = currentTime - lastTime;		// Calculate delta time
        lastTime = currentTime;					// Save for next frame calculations.
        glfwPollEvents();						// poll callbacks
        update(currentTime);					// update (physics, animation, structures, etc)
        render(currentTime);					// call render function.

        glfwSwapBuffers(window);				// swap buffers (avoid flickering and tearing)

        running &= (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE);	// exit if escape key pressed
        running &= (glfwWindowShouldClose(window) != GL_TRUE);
    } while (running);

    endProgram();			// Close and clean everything up...

    cout << "\nPress any key to continue...\n";
    cin.ignore(); cin.get(); // delay closing console to read debugging errors.

    return 0;
}

void errorCallbackGLFW(int error, const char* description) {
    cout << "Error GLFW: " << description << "\n";
}

void hintsGLFW() {
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);			// Create context in debug mode - for debug message callback
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
}

void endProgram() {
    glfwMakeContextCurrent(window);		// destroys window handler
    glfwTerminate();	// destroys all windows and releases resources.

                        // tidy heap memory
    delete[] rockStruct.texture;
    delete[] groundStruct.texture;
    delete[] towerStruct.texture;
    delete[] lightModel.texture;
    delete[] fanStruct.texture;
    delete[] rockPositions;
    delete[] rockRotations;
}

void setupRender() {
    glfwSwapInterval(1);	// Ony render when synced (V SYNC)

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_STEREO, GL_FALSE);
}

void debugGL() {
    //Output some debugging information
    cout << "VENDOR: " << (char *)glGetString(GL_VENDOR) << endl;
    cout << "VERSION: " << (char *)glGetString(GL_VERSION) << endl;
    cout << "RENDERER: " << (char *)glGetString(GL_RENDERER) << endl;

    // Enable Opengl Debug
    //glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback((GLDEBUGPROC)openGLDebugCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
}

static void APIENTRY openGLDebugCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const GLvoid* userParam) {

    cout << "---------------------opengl-callback------------" << endl;
    cout << "Message: " << message << endl;
    cout << "type: ";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        cout << "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        cout << "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        cout << "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        cout << "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        cout << "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        cout << "OTHER";
        break;
    }
    cout << " --- ";

    cout << "id: " << id << " --- ";
    cout << "severity: ";
    switch (severity) {
    case GL_DEBUG_SEVERITY_LOW:
        cout << "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        cout << "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        cout << "HIGH";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        cout << "NOTIFICATION";
    }
    cout << endl;
    cout << "-----------------------------------------" << endl;
}

#pragma endregion

void startup() {

    // Load main object model and shaders

#pragma region rock
    rockStruct.program = glCreateProgram();

    string vs_text = readShader("vs_model.glsl"); static const char* vs_source = vs_text.c_str();
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_source, NULL);
    glCompileShader(vs);
    checkErrorShader(vs);
    glAttachShader(rockStruct.program, vs);

    string fs_text = readShader("fs_model.glsl"); static const char* fs_source = fs_text.c_str();
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_source, NULL);
    glCompileShader(fs);
    checkErrorShader(fs);
    glAttachShader(rockStruct.program, fs);

    glLinkProgram(rockStruct.program);
    glUseProgram(rockStruct.program);


    readObj("bube_rock.obj", &rockStruct);

    //OPENGL 4.3
    glGenVertexArrays(1, &rockStruct.vao);			// Create Vertex Array Object
    glBindVertexArray(rockStruct.vao);				// Bind VertexArrayObject

    glGenBuffers(3, rockStruct.buffer);			// Create new buffers (Vertices, Texture Coordinates, Normals
    glBindBuffer(GL_ARRAY_BUFFER, rockStruct.buffer[0]);	// Bind Buffer Vertex
    glBufferStorage(GL_ARRAY_BUFFER, rockStruct.out_vertices.size() * sizeof(glm::vec3), &rockStruct.out_vertices[0], GL_DYNAMIC_STORAGE_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, rockStruct.buffer[1]);	// Bind Buffer UV
    glBufferStorage(GL_ARRAY_BUFFER, rockStruct.out_uvs.size() * sizeof(glm::vec2), &rockStruct.out_uvs[0], GL_DYNAMIC_STORAGE_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, rockStruct.buffer[2]);	// Bind Buffer Normals
    glBufferStorage(GL_ARRAY_BUFFER, rockStruct.out_normals.size() * sizeof(glm::vec3), &rockStruct.out_normals[0], GL_DYNAMIC_STORAGE_BIT);

    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);		// Vertices									
    glBindVertexBuffer(0, rockStruct.buffer[0], 0, sizeof(GLfloat) * 3);
    glVertexAttribBinding(0, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 0);			// UV									
    glBindVertexBuffer(1, rockStruct.buffer[1], 0, sizeof(GLfloat) * 2);
    glVertexAttribBinding(1, 1);
    glEnableVertexAttribArray(1);

    glVertexAttribFormat(2, 3, GL_FLOAT, GL_FALSE, 0);			// Normals									
    glBindVertexBuffer(2, rockStruct.buffer[2], 0, sizeof(GLfloat) * 3);
    glVertexAttribBinding(2, 2);
    glEnableVertexAttribArray(2);


    rockStruct.mv_location = glGetUniformLocation(rockStruct.program, "mv_matrix");
    rockStruct.proj_location = glGetUniformLocation(rockStruct.program, "proj_matrix");
    rockStruct.tex_location = glGetUniformLocation(rockStruct.program, "tex");
    rockStruct.lightColor_location = glGetUniformLocation(rockStruct.program, "ia");
    rockStruct.lightColor_location = glGetUniformLocation(rockStruct.program, "ka");

    rockPositions = new glm::vec3[12];
    rockPositions[0] = glm::vec3(0.0f, 0.0f, 0.0f);
    rockPositions[1] = glm::vec3(-0.5f, 0.0f, -0.5f);
    rockPositions[2] = glm::vec3(1.0f, 0.0f, -1.0f);
    rockPositions[3] = glm::vec3(0.5f, 0.0f, -1.5f);
    rockPositions[4] = glm::vec3(-1.0f, 0.0f, -0.5f);
    rockPositions[5] = glm::vec3(1.0f, 0.0f, -0.0f);
    rockPositions[6] = glm::vec3(0.0f, 0.0f, 0.5f);
    rockPositions[7] = glm::vec3(0.5f, 0.0f, 1.0f);
    rockPositions[8] = glm::vec3(-1.0f, 0.0f, 1.5f);
    rockPositions[9] = glm::vec3(-0.5f, 0.0f, 1.0f);
    rockPositions[10] = glm::vec3(-1.0f, 0.0f, 0.5f);
    rockPositions[11] = glm::vec3(-1.0f, 0.0f, 0.0f);

    rockRotations = new glm::vec3[12];
    rockRotations[0] = glm::vec3(0.0f, 20.0f, 0.0f);
    rockRotations[1] = glm::vec3(0.0f, 30.0f, 0.0f);
    rockRotations[2] = glm::vec3(0.0f, 40.0f, 0.0f);
    rockRotations[3] = glm::vec3(0.0f, 80.0f, 0.0f);
    rockRotations[4] = glm::vec3(0.0f, 10.0f, 0.0f);
    rockRotations[5] = glm::vec3(0.0f, 50.0f, 0.0f);
    rockRotations[6] = glm::vec3(0.0f, 60.0f, 0.0f);
    rockRotations[7] = glm::vec3(0.0f, 90.0f, 0.0f);
    rockRotations[8] = glm::vec3(0.0f, 70.0f, 0.0f);
    rockRotations[9] = glm::vec3(0.0f, 60.0f, 0.0f);
    rockRotations[10] = glm::vec3(0.0f, 30.0f, 0.0f);
    rockRotations[11] = glm::vec3(0.0f, 20.0f, 0.0f);
#pragma endregion

#pragma region ground
    groundStruct.program = glCreateProgram();

    glAttachShader(groundStruct.program, vs);
    glAttachShader(groundStruct.program, fs);

    glLinkProgram(groundStruct.program);
    glUseProgram(groundStruct.program);


    readObj("cube_uv.obj", &groundStruct);

    //OPENGL 4.3
    glGenVertexArrays(1, &groundStruct.vao);			// Create Vertex Array Object
    glBindVertexArray(groundStruct.vao);				// Bind VertexArrayObject

    glGenBuffers(3, groundStruct.buffer);			// Create new buffers (Vertices, Texture Coordinates, Normals
    glBindBuffer(GL_ARRAY_BUFFER, groundStruct.buffer[0]);	// Bind Buffer Vertex
    glBufferStorage(GL_ARRAY_BUFFER, groundStruct.out_vertices.size() * sizeof(glm::vec3), &groundStruct.out_vertices[0], GL_DYNAMIC_STORAGE_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, groundStruct.buffer[1]);	// Bind Buffer UV
    glBufferStorage(GL_ARRAY_BUFFER, groundStruct.out_uvs.size() * sizeof(glm::vec2), &groundStruct.out_uvs[0], GL_DYNAMIC_STORAGE_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, groundStruct.buffer[2]);	// Bind Buffer Normals
    glBufferStorage(GL_ARRAY_BUFFER, groundStruct.out_normals.size() * sizeof(glm::vec3), &groundStruct.out_normals[0], GL_DYNAMIC_STORAGE_BIT);

    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);		// Vertices									
    glBindVertexBuffer(0, groundStruct.buffer[0], 0, sizeof(GLfloat) * 3);
    glVertexAttribBinding(0, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 0);			// UV									
    glBindVertexBuffer(1, groundStruct.buffer[1], 0, sizeof(GLfloat) * 2);
    glVertexAttribBinding(1, 1);
    glEnableVertexAttribArray(1);

    glVertexAttribFormat(2, 3, GL_FLOAT, GL_FALSE, 0);			// Normals									
    glBindVertexBuffer(2, groundStruct.buffer[2], 0, sizeof(GLfloat) * 3);
    glVertexAttribBinding(2, 2);
    glEnableVertexAttribArray(2);


    groundStruct.mv_location = glGetUniformLocation(groundStruct.program, "mv_matrix");
    groundStruct.proj_location = glGetUniformLocation(groundStruct.program, "proj_matrix");
    groundStruct.tex_location = glGetUniformLocation(groundStruct.program, "tex");
    groundStruct.lightColor_location = glGetUniformLocation(groundStruct.program, "ia");
    groundStruct.lightColor_location = glGetUniformLocation(groundStruct.program, "ka");
    
#pragma endregion

#pragma region tower
    towerStruct.program = glCreateProgram();

    glAttachShader(towerStruct.program, vs);
    glAttachShader(towerStruct.program, fs);

    glLinkProgram(towerStruct.program);
    glUseProgram(towerStruct.program);


    readObj("tower.obj", &towerStruct);

    //OPENGL 4.3
    glGenVertexArrays(1, &towerStruct.vao);			// Create Vertex Array Object
    glBindVertexArray(towerStruct.vao);				// Bind VertexArrayObject

    glGenBuffers(3, towerStruct.buffer);			// Create new buffers (Vertices, Texture Coordinates, Normals
    glBindBuffer(GL_ARRAY_BUFFER, towerStruct.buffer[0]);	// Bind Buffer Vertex
    glBufferStorage(GL_ARRAY_BUFFER, towerStruct.out_vertices.size() * sizeof(glm::vec3), &towerStruct.out_vertices[0], GL_DYNAMIC_STORAGE_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, towerStruct.buffer[1]);	// Bind Buffer UV
    glBufferStorage(GL_ARRAY_BUFFER, towerStruct.out_uvs.size() * sizeof(glm::vec2), &towerStruct.out_uvs[0], GL_DYNAMIC_STORAGE_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, towerStruct.buffer[2]);	// Bind Buffer Normals
    glBufferStorage(GL_ARRAY_BUFFER, towerStruct.out_normals.size() * sizeof(glm::vec3), &towerStruct.out_normals[0], GL_DYNAMIC_STORAGE_BIT);

    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);		// Vertices									
    glBindVertexBuffer(0, towerStruct.buffer[0], 0, sizeof(GLfloat) * 3);
    glVertexAttribBinding(0, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 0);			// UV									
    glBindVertexBuffer(1, towerStruct.buffer[1], 0, sizeof(GLfloat) * 2);
    glVertexAttribBinding(1, 1);
    glEnableVertexAttribArray(1);

    glVertexAttribFormat(2, 3, GL_FLOAT, GL_FALSE, 0);			// Normals									
    glBindVertexBuffer(2, towerStruct.buffer[2], 0, sizeof(GLfloat) * 3);
    glVertexAttribBinding(2, 2);
    glEnableVertexAttribArray(2);


    towerStruct.mv_location = glGetUniformLocation(towerStruct.program, "mv_matrix");
    towerStruct.proj_location = glGetUniformLocation(towerStruct.program, "proj_matrix");
    towerStruct.tex_location = glGetUniformLocation(towerStruct.program, "tex");
    towerStruct.lightColor_location = glGetUniformLocation(towerStruct.program, "ia");
    towerStruct.lightColor_location = glGetUniformLocation(towerStruct.program, "ka");

#pragma endregion

#pragma region fan
    fanStruct.program = glCreateProgram();

    glAttachShader(fanStruct.program, vs);
    glAttachShader(fanStruct.program, fs);

    glLinkProgram(fanStruct.program);
    glUseProgram(fanStruct.program);


    readObj("fan.obj", &fanStruct);

    //OPENGL 4.3
    glGenVertexArrays(1, &fanStruct.vao);			// Create Vertex Array Object
    glBindVertexArray(fanStruct.vao);				// Bind VertexArrayObject

    glGenBuffers(3, fanStruct.buffer);			// Create new buffers (Vertices, Texture Coordinates, Normals
    glBindBuffer(GL_ARRAY_BUFFER, fanStruct.buffer[0]);	// Bind Buffer Vertex
    glBufferStorage(GL_ARRAY_BUFFER, fanStruct.out_vertices.size() * sizeof(glm::vec3), &fanStruct.out_vertices[0], GL_DYNAMIC_STORAGE_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, fanStruct.buffer[1]);	// Bind Buffer UV
    glBufferStorage(GL_ARRAY_BUFFER, fanStruct.out_uvs.size() * sizeof(glm::vec2), &fanStruct.out_uvs[0], GL_DYNAMIC_STORAGE_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, fanStruct.buffer[2]);	// Bind Buffer Normals
    glBufferStorage(GL_ARRAY_BUFFER, fanStruct.out_normals.size() * sizeof(glm::vec3), &fanStruct.out_normals[0], GL_DYNAMIC_STORAGE_BIT);

    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);		// Vertices									
    glBindVertexBuffer(0, fanStruct.buffer[0], 0, sizeof(GLfloat) * 3);
    glVertexAttribBinding(0, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 0);			// UV									
    glBindVertexBuffer(1, fanStruct.buffer[1], 0, sizeof(GLfloat) * 2);
    glVertexAttribBinding(1, 1);
    glEnableVertexAttribArray(1);

    glVertexAttribFormat(2, 3, GL_FLOAT, GL_FALSE, 0);			// Normals									
    glBindVertexBuffer(2, fanStruct.buffer[2], 0, sizeof(GLfloat) * 3);
    glVertexAttribBinding(2, 2);
    glEnableVertexAttribArray(2);


    fanStruct.mv_location = glGetUniformLocation(fanStruct.program, "mv_matrix");
    fanStruct.proj_location = glGetUniformLocation(fanStruct.program, "proj_matrix");
    fanStruct.tex_location = glGetUniformLocation(fanStruct.program, "tex");
    fanStruct.lightColor_location = glGetUniformLocation(fanStruct.program, "ia");
    fanStruct.lightColor_location = glGetUniformLocation(fanStruct.program, "ka");

#pragma endregion

#pragma region light

    lightModel.program = glCreateProgram();

    string vs_textLight = readShader("vs_light.glsl"); static const char* vs_sourceLight = vs_textLight.c_str();
    GLuint vsLight = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsLight, 1, &vs_sourceLight, NULL);
    glCompileShader(vsLight);
    checkErrorShader(vsLight);
    glAttachShader(lightModel.program, vsLight);

    string fs_textLight = readShader("fs_light.glsl"); static const char* fs_sourceLight = fs_textLight.c_str();
    GLuint fsLight = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsLight, 1, &fs_sourceLight, NULL);
    glCompileShader(fsLight);
    checkErrorShader(fsLight);
    glAttachShader(lightModel.program, fsLight);

    glLinkProgram(lightModel.program);

    readObj("light.obj", &lightModel);
    //readObj("cube_uv.obj", &lightModel);

    glGenVertexArrays(1, &lightModel.vao);			// Create Vertex Array Object
    glBindVertexArray(lightModel.vao);				// Bind VertexArrayObject

    glGenBuffers(3, lightModel.buffer);			// Create new buffers (Vertices, Texture Coordinates, Normals
    glBindBuffer(GL_ARRAY_BUFFER, lightModel.buffer[0]);	// Bind Buffer Vertex
    glBufferStorage(GL_ARRAY_BUFFER, lightModel.out_vertices.size() * sizeof(glm::vec3), &lightModel.out_vertices[0], GL_DYNAMIC_STORAGE_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, lightModel.buffer[1]);	// Bind Buffer UV
    glBufferStorage(GL_ARRAY_BUFFER, lightModel.out_uvs.size() * sizeof(glm::vec2), &lightModel.out_uvs[0], GL_DYNAMIC_STORAGE_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, lightModel.buffer[2]);	// Bind Buffer Normals
    glBufferStorage(GL_ARRAY_BUFFER, lightModel.out_normals.size() * sizeof(glm::vec3), &lightModel.out_normals[0], GL_DYNAMIC_STORAGE_BIT);

    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);		// Vertices									
    glBindVertexBuffer(0, lightModel.buffer[0], 0, sizeof(GLfloat) * 3);
    glVertexAttribBinding(0, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 0);			// UV									
    glBindVertexBuffer(1, lightModel.buffer[1], 0, sizeof(GLfloat) * 2);
    glVertexAttribBinding(1, 1);
    glEnableVertexAttribArray(1);

    glVertexAttribFormat(2, 3, GL_FLOAT, GL_FALSE, 0);			// Normals									
    glBindVertexBuffer(2, lightModel.buffer[2], 0, sizeof(GLfloat) * 3);
    glVertexAttribBinding(2, 2);
    glEnableVertexAttribArray(2);


    lightModel.mv_location = glGetUniformLocation(lightModel.program, "mv_matrix");
    lightModel.proj_location = glGetUniformLocation(lightModel.program, "proj_matrix");
    lightModel.tex_location = glGetUniformLocation(lightModel.program, "tex");
#pragma endregion

    /*
#pragma region Framebuffer
    fbStruct.program = glCreateProgram();
    
    // shaders
    string vs_textframeBuffer = readShader("vs_framebuffer.glsl"); static const char* vs_frameBuffer = vs_textframeBuffer.c_str();
    GLuint vsFB = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsFB, 1, &vs_frameBuffer, NULL);
    glCompileShader(vsFB);
    checkErrorShader(vsFB);
    glAttachShader(fbStruct.program, vsFB);

    string fs_textframeBuffer = readShader("fs_framebuffer.glsl"); static const char* fs_frameBuffer = fs_textframeBuffer.c_str();
    GLuint fsFB = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fsFB, 1, &fs_frameBuffer, NULL);
    glCompileShader(fsFB);
    checkErrorShader(fsFB);
    glAttachShader(fbStruct.program, fsFB);

    glLinkProgram(fbStruct.program);

    // vertices
    glGenVertexArrays(1, &fbStruct.vao);
    glBindVertexArray(fbStruct.vao);
    //glBindBuffer(GL_ARRAY_BUFFER, fbStruct.buffer[0]);	// Bind Buffer Vertex
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    // framebuffer configuration
    glGenFramebuffers(1, &fbStruct.buffer[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, fbStruct.buffer[0]);
    // create a color attachment texture
    glGenTextures(1, &fbStruct.buffer[1]);
    glBindTexture(GL_TEXTURE_2D, fbStruct.buffer[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbStruct.buffer[1], 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    //unsigned int rbo;
    glGenRenderbuffers(1, &fbStruct.buffer[2]);
    glBindRenderbuffer(GL_RENDERBUFFER, fbStruct.buffer[2]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbStruct.buffer[2]); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);*/

#pragma endregion

    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Calculate proj_matrix for the first time.
    aspect = (float)windowWidth / (float)windowHeight;
    proj_matrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
}

void update(GLfloat currentTime) {

    // calculate movement
    GLfloat cameraSpeed = 1.0f * deltaTime;
    if (keyStatus[GLFW_KEY_W]) cameraPosition += cameraSpeed * cameraFront;
    if (keyStatus[GLFW_KEY_S]) cameraPosition -= cameraSpeed * cameraFront;
    if (keyStatus[GLFW_KEY_A]) cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (keyStatus[GLFW_KEY_D]) cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    if (keyStatus[GLFW_KEY_L] && (movingLight == false)) {
        cout << "Change mode to moving light...\n";
        movingLight = true;
    }
    if (keyStatus[GLFW_KEY_M] && (movingLight == true)) {
        cout << "Change mode to moving object...\n";
        movingLight = false;
    }

    if (movingLight == false) {		// moving object rotation and z displacement
        if (keyStatus[GLFW_KEY_LEFT])			modelAngle.y += 0.05f;
        if (keyStatus[GLFW_KEY_RIGHT])			modelAngle.y -= 0.05f;
        if (keyStatus[GLFW_KEY_UP])				modelAngle.x += 0.05f;
        if (keyStatus[GLFW_KEY_DOWN])			modelAngle.x -= 0.05f;
        if (keyStatus[GLFW_KEY_KP_ADD])			modelDisp.z += 0.10f;
        if (keyStatus[GLFW_KEY_KP_SUBTRACT])	modelDisp.z -= 0.10f;
    }
    else {							// moving light displacement x y z
        if (keyStatus[GLFW_KEY_LEFT])			lightDisp.x -= 0.05f;
        if (keyStatus[GLFW_KEY_RIGHT])			lightDisp.x += 0.05f;
        if (keyStatus[GLFW_KEY_UP])				lightDisp.y += 0.05f;
        if (keyStatus[GLFW_KEY_DOWN])			lightDisp.y -= 0.05f;
        if (keyStatus[GLFW_KEY_KP_ADD])			lightDisp.z += 0.05f;
        if (keyStatus[GLFW_KEY_KP_SUBTRACT])	lightDisp.z -= 0.05f;
    }
}

void render(GLfloat currentTime) {
    //framebuffer
    /*glBindFramebuffer(GL_FRAMEBUFFER, fbStruct.buffer[0]);
    glEnable(GL_DEPTH_TEST);*/
    
    glViewport(0, 0, windowWidth, windowHeight);

    // Clear colour buffer
    glm::vec4 backgroundColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f); glClearBufferfv(GL_COLOR, 0, &backgroundColor[0]);

    // Clear Deep buffer
    static const GLfloat one = 1.0f; glClearBufferfv(GL_DEPTH, 0, &one);

    // Enable blend
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 viewMatrix = glm::lookAt(cameraPosition,					// eye
        cameraPosition + cameraFront,	// centre
        cameraUp);						// up

#pragma region rocks

    glUseProgram(rockStruct.program);
    glBindVertexArray(rockStruct.vao);
    glUniformMatrix4fv(rockStruct.proj_location, 1, GL_FALSE, &proj_matrix[0][0]);

    glUniform4f(glGetUniformLocation(rockStruct.program, "viewPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0);
    glUniform4f(glGetUniformLocation(rockStruct.program, "lightPosition"), lightDisp.x, lightDisp.y, lightDisp.z, 1.0);
    glUniform4f(glGetUniformLocation(rockStruct.program, "ia"), ia.r, ia.g, ia.b, 1.0);
    glUniform1f(glGetUniformLocation(rockStruct.program, "ka"), ka);
    glUniform4f(glGetUniformLocation(rockStruct.program, "id"), id.r, id.g, id.b, 1.0);
    glUniform1f(glGetUniformLocation(rockStruct.program, "kd"), 1.0f);
    glUniform4f(glGetUniformLocation(rockStruct.program, "is"), is.r, is.g, is.b, 1.0);
    glUniform1f(glGetUniformLocation(rockStruct.program, "ks"), 1.0f);
    glUniform1f(glGetUniformLocation(rockStruct.program, "shininess"), 32.0f);


    // Bind textures and samplers - using 0 as I know there is only one texture - need to extend.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rockStruct.texture[0]);
    glUniform1i(rockStruct.tex_location, 0);

    for (int i = 0; i < 12; i++) {
        glm::mat4 modelMatrixRocks = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));// modelDisp.z));
        modelMatrixRocks = glm::translate(modelMatrixRocks, rockPositions[i]);
        modelMatrixRocks = glm::rotate(modelMatrixRocks, modelAngle.x + rockRotations[i].x, glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrixRocks = glm::rotate(modelMatrixRocks, modelAngle.y + rockRotations[i].y, glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrixRocks = glm::scale(modelMatrixRocks, glm::vec3(0.2f, 0.2f, 0.2f));

        glm::mat4 mv_matrix = viewMatrix * modelMatrixRocks;

        glUniformMatrix4fv(glGetUniformLocation(rockStruct.program, "model_matrix"), 1, GL_FALSE, &modelMatrixRocks[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(rockStruct.program, "view_matrix"), 1, GL_FALSE, &viewMatrix[0][0]);

        glDrawArrays(GL_TRIANGLES, 0, rockStruct.out_vertices.size());

    }
#pragma endregion

#pragma region ground

    glUseProgram(groundStruct.program);
    glBindVertexArray(groundStruct.vao);
    glUniformMatrix4fv(groundStruct.proj_location, 1, GL_FALSE, &proj_matrix[0][0]);

    glUniform4f(glGetUniformLocation(groundStruct.program, "viewPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0);
    glUniform4f(glGetUniformLocation(groundStruct.program, "lightPosition"), lightDisp.x, lightDisp.y, lightDisp.z, 1.0);
    glUniform4f(glGetUniformLocation(groundStruct.program, "ia"), ia.r, ia.g, ia.b, 1.0);
    glUniform1f(glGetUniformLocation(groundStruct.program, "ka"), ka);
    glUniform4f(glGetUniformLocation(groundStruct.program, "id"), id.r, id.g, id.b, 1.0);
    glUniform1f(glGetUniformLocation(groundStruct.program, "kd"), 1.0f);
    glUniform4f(glGetUniformLocation(groundStruct.program, "is"), is.r, is.g, is.b, 1.0);
    glUniform1f(glGetUniformLocation(groundStruct.program, "ks"), 1.0f);
    glUniform1f(glGetUniformLocation(groundStruct.program, "shininess"), 32.0f);


    // Bind textures and samplers - using 0 as I know there is only one texture - need to extend.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, groundStruct.texture[0]);
    glUniform1i(groundStruct.tex_location, 0);

    glm::mat4 modelMatrixGround = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
        
    modelMatrixGround = glm::scale(modelMatrixGround, glm::vec3(8.0f, 0.5f, 8.0f));

    glm::mat4 mv_matrix = viewMatrix * modelMatrixGround;

    glUniformMatrix4fv(glGetUniformLocation(groundStruct.program, "model_matrix"), 1, GL_FALSE, &modelMatrixGround[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(groundStruct.program, "view_matrix"), 1, GL_FALSE, &viewMatrix[0][0]);

    glDrawArrays(GL_TRIANGLES, 0, groundStruct.out_vertices.size());

#pragma endregion

#pragma region tower

    glUseProgram(towerStruct.program);
    glBindVertexArray(towerStruct.vao);
    glUniformMatrix4fv(towerStruct.proj_location, 1, GL_FALSE, &proj_matrix[0][0]);

    glUniform4f(glGetUniformLocation(towerStruct.program, "viewPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0);
    glUniform4f(glGetUniformLocation(towerStruct.program, "lightPosition"), lightDisp.x, lightDisp.y, lightDisp.z, 1.0);
    glUniform4f(glGetUniformLocation(towerStruct.program, "ia"), ia.r, ia.g, ia.b, 1.0);
    glUniform1f(glGetUniformLocation(towerStruct.program, "ka"), ka);
    glUniform4f(glGetUniformLocation(towerStruct.program, "id"), id.r, id.g, id.b, 1.0);
    glUniform1f(glGetUniformLocation(towerStruct.program, "kd"), 1.0f);
    glUniform4f(glGetUniformLocation(towerStruct.program, "is"), is.r, is.g, is.b, 1.0);
    glUniform1f(glGetUniformLocation(towerStruct.program, "ks"), 1.0f);
    glUniform1f(glGetUniformLocation(towerStruct.program, "shininess"), 32.0f);


    // Bind textures and samplers - using 0 as I know there is only one texture - need to extend.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, towerStruct.texture[0]);
    glUniform1i(towerStruct.tex_location, 0);

    glm::mat4 modelMatrixTower = glm::translate(glm::mat4(1.0f), glm::vec3(-0.2f, 0.6f, -1.2f));

    modelMatrixTower = glm::scale(modelMatrixTower, glm::vec3(.3f, 0.6f, .3f));

    glm::mat4 mv_matrixTower = viewMatrix * modelMatrixTower;

    glUniformMatrix4fv(glGetUniformLocation(towerStruct.program, "model_matrix"), 1, GL_FALSE, &modelMatrixTower[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(towerStruct.program, "view_matrix"), 1, GL_FALSE, &viewMatrix[0][0]);

    glDrawArrays(GL_TRIANGLES, 0, towerStruct.out_vertices.size());

#pragma endregion

#pragma region fan

    glUseProgram(fanStruct.program);
    glBindVertexArray(fanStruct.vao);
    glUniformMatrix4fv(fanStruct.proj_location, 1, GL_FALSE, &proj_matrix[0][0]);

    glUniform4f(glGetUniformLocation(fanStruct.program, "viewPosition"), cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0);
    glUniform4f(glGetUniformLocation(fanStruct.program, "lightPosition"), lightDisp.x, lightDisp.y, lightDisp.z, 1.0);
    glUniform4f(glGetUniformLocation(fanStruct.program, "ia"), ia.r, ia.g, ia.b, 1.0);
    glUniform1f(glGetUniformLocation(fanStruct.program, "ka"), ka);
    glUniform4f(glGetUniformLocation(fanStruct.program, "id"), id.r, id.g, id.b, 1.0);
    glUniform1f(glGetUniformLocation(fanStruct.program, "kd"), 1.0f);
    glUniform4f(glGetUniformLocation(fanStruct.program, "is"), is.r, is.g, is.b, 1.0);
    glUniform1f(glGetUniformLocation(fanStruct.program, "ks"), 1.0f);
    glUniform1f(glGetUniformLocation(fanStruct.program, "shininess"), 32.0f);


    // Bind textures and samplers - using 0 as I know there is only one texture - need to extend.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fanStruct.texture[0]);
    glUniform1i(fanStruct.tex_location, 0);

    glm::mat4 modelMatrixFan = glm::translate(glm::mat4(1.0f), glm::vec3(-0.2f, 0.9f, -0.85f));
    modelMatrixFan = glm::rotate(modelMatrixFan, currentTime, glm::vec3(0.0f, 0.0f, 1.0f));
    modelMatrixFan = glm::scale(modelMatrixFan, glm::vec3(.1f, 0.5f, .05f));

    //I think this line is useless, this is calculated in the shader
    glm::mat4 mv_matrixFan = viewMatrix * modelMatrixFan;

    glUniformMatrix4fv(glGetUniformLocation(fanStruct.program, "model_matrix"), 1, GL_FALSE, &modelMatrixFan[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(fanStruct.program, "view_matrix"), 1, GL_FALSE, &viewMatrix[0][0]);

    glDrawArrays(GL_TRIANGLES, 0, fanStruct.out_vertices.size());

#pragma endregion

    // ----------draw light------------
    glUseProgram(lightModel.program);
    glBindVertexArray(lightModel.vao);
    glUniformMatrix4fv(lightModel.proj_location, 1, GL_FALSE, &proj_matrix[0][0]);

    // Bind textures and samplers - using 0 as I know there is only one texture - need to extend.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lightModel.texture[0]);
    glUniform1i(lightModel.tex_location, 0);

    glm::mat4 modelMatrixLight = glm::translate(glm::mat4(1.0f), glm::vec3(lightDisp.x, lightDisp.y, lightDisp.z));
    modelMatrixLight = glm::scale(modelMatrixLight, glm::vec3(0.2f, 0.1f, 0.1f));
    glm::mat4 mv_matrixLight = viewMatrix * modelMatrixLight;

    glUniformMatrix4fv(lightModel.mv_location, 1, GL_FALSE, &mv_matrixLight[0][0]);

    glDrawArrays(GL_TRIANGLES, 0, lightModel.out_vertices.size());

    //framebuffer
    /*
    // now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
    // clear all relevant buffers
    glClearColor(.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)
    glClear(GL_COLOR_BUFFER_BIT);


    glUseProgram(fbStruct.program);
    glBindVertexArray(fbStruct.vao);
    
    glBindTexture(GL_TEXTURE_2D, fbStruct.buffer[1]);	// use the color attachment texture as the texture of the quad plane
    glDrawArrays(GL_TRIANGLES, 0, 6);*/

}

#pragma region Callbacks

void onResizeCallback(GLFWwindow* window, int w, int h) {
    windowWidth = w;
    windowHeight = h;

    aspect = (float)w / (float)h;
    proj_matrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
}

void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) keyStatus[key] = true;
    else if (action == GLFW_RELEASE) keyStatus[key] = false;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

}

void onMouseMoveCallback(GLFWwindow* window, double x, double y) {
    int mouseX = static_cast<int>(x);
    int mouseY = static_cast<int>(y);

    if (firstMouse) {
        lastX = (GLfloat)mouseX; lastY = (GLfloat)mouseY; firstMouse = false;
    }

    GLfloat xoffset = mouseX - lastX;
    GLfloat yoffset = lastY - mouseY; // Reversed
    lastX = (GLfloat)mouseX; lastY = (GLfloat)mouseY;

    GLfloat sensitivity = 0.05f;
    xoffset *= sensitivity; yoffset *= sensitivity;

    yaw += xoffset; pitch += yoffset;

    // check for pitch out of bounds otherwise screen gets flipped
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    cameraFront = glm::normalize(front);

}

static void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset) {
    int yoffsetInt = static_cast<int>(yoffset);

    fovy += yoffsetInt;
    if (fovy >= 1.0f && fovy <= 45.0f) fovy -= (GLfloat)yoffset;
    if (fovy <= 1.0f) fovy = 1.0f;
    if (fovy >= 45.0f) fovy = 45.0f;
    proj_matrix = glm::perspective(glm::radians(fovy), aspect, 0.1f, 1000.0f);
}

#pragma endregion

#pragma region file i/o

string readShader(string name) {
    string vs_text;
    std::ifstream vs_file(name);

    string vs_line;
    if (vs_file.is_open()) {

        while (getline(vs_file, vs_line)) {
            vs_text += vs_line;
            vs_text += '\n';
        }
        vs_file.close();
    }
    return vs_text;
}

void  checkErrorShader(GLuint shader) {
    // Get log lenght
    GLint maxLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    // Init a string for it
    std::vector<GLchar> errorLog(maxLength);

    if (maxLength > 1) {
        // Get the log file
        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

        cout << "--------------Shader compilation error-------------\n";
        cout << errorLog.data();
    }

}

void readObj(string name, struct modelObject *obj) {
    cout << "Loading model " << name << "\n";

    std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
    std::vector< string > materials, textures;
    std::vector< glm::vec3 > obj_vertices;
    std::vector< glm::vec2 > obj_uvs;
    std::vector< glm::vec3 > obj_normals;

    std::ifstream dataFile(name);

    string rawData;		// store the raw data.
    int countLines = 0;
    if (dataFile.is_open()) {
        string dataLineRaw;
        while (getline(dataFile, dataLineRaw)) {
            rawData += dataLineRaw;
            rawData += "\n";
            countLines++;
        }
        dataFile.close();
    }

    cout << "Finished reading model file " << name << "\n";

    istringstream rawDataStream(rawData);
    string dataLine;
    int linesDone = 0;
    while (std::getline(rawDataStream, dataLine)) {
        if (dataLine.find("v ") != string::npos) {	// does this line have a vector?
            glm::vec3 vertex;

            int foundStart = dataLine.find(" ");  int foundEnd = dataLine.find(" ", foundStart + 1);
            vertex.x = stof(dataLine.substr(foundStart, foundEnd - foundStart));

            foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
            vertex.y = stof(dataLine.substr(foundStart, foundEnd - foundStart));

            foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
            vertex.z = stof(dataLine.substr(foundStart, foundEnd - foundStart));

            obj_vertices.push_back(vertex);
        }
        else if (dataLine.find("vt ") != string::npos) {	// does this line have a uv coordinates?
            glm::vec2 uv;

            int foundStart = dataLine.find(" ");  int foundEnd = dataLine.find(" ", foundStart + 1);
            uv.x = stof(dataLine.substr(foundStart, foundEnd - foundStart));

            foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
            uv.y = stof(dataLine.substr(foundStart, foundEnd - foundStart));

            obj_uvs.push_back(uv);
        }
        else if (dataLine.find("vn ") != string::npos) { // does this line have a normal coordinates?
            glm::vec3 normal;

            int foundStart = dataLine.find(" ");  int foundEnd = dataLine.find(" ", foundStart + 1);
            normal.x = stof(dataLine.substr(foundStart, foundEnd - foundStart));

            foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
            normal.y = stof(dataLine.substr(foundStart, foundEnd - foundStart));

            foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
            normal.z = stof(dataLine.substr(foundStart, foundEnd - foundStart));

            obj_normals.push_back(normal);
        }
        else if (dataLine.find("f ") != string::npos) { // does this line defines a triangle face?
            string parts[3];

            int foundStart = dataLine.find(" ");  int foundEnd = dataLine.find(" ", foundStart + 1);
            parts[0] = dataLine.substr(foundStart + 1, foundEnd - foundStart - 1);

            foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
            parts[1] = dataLine.substr(foundStart + 1, foundEnd - foundStart - 1);

            foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
            parts[2] = dataLine.substr(foundStart + 1, foundEnd - foundStart - 1);

            for (int i = 0; i < 3; i++) {		// for each part

                vertexIndices.push_back(stoul(parts[i].substr(0, parts[i].find("/"))));

                int firstSlash = parts[i].find("/"); int secondSlash = parts[i].find("/", firstSlash + 1);

                if ((firstSlash + 1) != (secondSlash)) {	// there are texture coordinates.
                    uvIndices.push_back(stoul(parts[i].substr(firstSlash + 1, secondSlash - firstSlash + 1)));
                }


                normalIndices.push_back(stoul(parts[i].substr(secondSlash + 1)));

            }
        }
        else if (dataLine.find("mtllib ") != string::npos) { // does this object have a material?
            materials.push_back(dataLine.substr(dataLine.find(" ") + 1));
        }

        linesDone++;

        if (linesDone % 50000 == 0) {
            cout << "Parsed " << linesDone << " of " << countLines << " from model...\n";
        }

    }

    // Double check here in which coordinate system you exported your models - and flip or not the vertices...
    /*
    for (unsigned int i = 0; i < vertexIndices.size(); i += 3) {
    (*obj).out_vertices.push_back(obj_vertices[vertexIndices[i+2] - 1]);
    (*obj).out_normals.push_back(obj_normals[normalIndices[i+2] - 1]);
    (*obj).out_uvs.push_back(obj_uvs[uvIndices[i+2] - 1]);

    (*obj).out_vertices.push_back(obj_vertices[vertexIndices[i+1] - 1]);
    (*obj).out_normals.push_back(obj_normals[normalIndices[i+1] - 1]);
    (*obj).out_uvs.push_back(obj_uvs[uvIndices[i + 1] - 1]);

    (*obj).out_vertices.push_back(obj_vertices[vertexIndices[i] - 1]);
    (*obj).out_normals.push_back(obj_normals[normalIndices[i] - 1]);
    (*obj).out_uvs.push_back(obj_uvs[uvIndices[i + 0] - 1]);
    }
    */

    for (unsigned int i = 0; i < vertexIndices.size(); i++) {
        (*obj).out_vertices.push_back(obj_vertices[vertexIndices[i] - 1]);
        (*obj).out_normals.push_back(obj_normals[normalIndices[i] - 1]);
        (*obj).out_uvs.push_back(obj_uvs[uvIndices[i] - 1]);
    }


    // Load Materials
    for (unsigned int i = 0; i < materials.size(); i++) {

        std::ifstream dataFileMat(materials[i]);

        string rawDataMat;		// store the raw data.
        int countLinesMat = 0;
        if (dataFileMat.is_open()) {
            string dataLineRawMat;
            while (getline(dataFileMat, dataLineRawMat)) {
                rawDataMat += dataLineRawMat;
                rawDataMat += "\n";
            }
            dataFileMat.close();
        }

        istringstream rawDataStreamMat(rawDataMat);
        string dataLineMat;
        while (std::getline(rawDataStreamMat, dataLineMat)) {
            if (dataLineMat.find("map_Kd ") != string::npos) {	// does this line have a texture map?
                textures.push_back(dataLineMat.substr(dataLineMat.find(" ") + 1));
            }
        }
    }

    (*obj).texture = new GLuint[textures.size()];		// Warning possible memory leak here - there is a new here, where is your delete?
    //glCreateTextures(GL_TEXTURE_2D, textures.size(), (*obj).texture);
    glGenTextures(textures.size(), (*obj).texture);

    for (unsigned int i = 0; i < textures.size(); i++) {
        readTexture(textures[i], (*obj).texture[i]);
    }

    cout << "done";
}

void readTexture(string name, GLuint textureName) {


    // Flip images as OpenGL expects 0.0 coordinates on the y-axis to be at the bottom of the image.
    stbi_set_flip_vertically_on_load(true);

    // Load image Information.
    int iWidth, iHeight, iChannels;
    unsigned char *iData = stbi_load(name.c_str(), &iWidth, &iHeight, &iChannels, 0);

    std::cout << iChannels;

    // Load and create a texture 
    glBindTexture(GL_TEXTURE_2D, textureName); // All upcoming operations now have effect on this texture object

    if (iChannels == 4) {
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, iWidth, iHeight);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iWidth, iHeight, GL_RGBA, GL_UNSIGNED_BYTE, iData);
    }
    if (iChannels == 3) {
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, iWidth, iHeight);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iWidth, iHeight, GL_RGB, GL_UNSIGNED_BYTE, iData);
    }


    // This only works for 2D Textures...
    // Set the texture wrapping parameters (next lecture)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Set texture filtering parameters (next lecture)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Generate mipmaps (next lecture)
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);// Unbind texture when done, so we won't accidentily mess up our texture.


}

#pragma endregion