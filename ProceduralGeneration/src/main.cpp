#include "utilities/global.h"
#include "utilities/console.h"
#include "utilities/input.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "graphics/mesh.h"
#include "graphics/camera.h"

#include "vendor/imgui/edited/ImGuiOpenGLRenderer.h"
#include "vendor/glm/glm.hpp"
#include "vendor/glm/gtc/matrix_transform.hpp"
#include "vendor/glm/gtx/euler_angles.hpp"

#include <vector>
#include <glfw3.h>
#include <glew.h>
#include <noise/noise.h>

#define TEXTURE_SIZE 1024

static void GLAPIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:
		Console::FatalError("GL CALLBACK:%s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? " ** GL ERROR **" : ""), type, severity, message);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		Console::Error("GL CALLBACK:%s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? " ** GL ERROR **" : ""), type, severity, message);
		break;
	case GL_DEBUG_SEVERITY_LOW:
		Console::Warning("GL CALLBACK:%s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? " ** GL ERROR **" : ""), type, severity, message);
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		Console::Info("GL NOTIFICATION:%s type = 0x%x, severity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? " ** GL ERROR **" : ""), type, severity, message);
		break;
	}
}

static double mouseXPos = 0.0;
static double mouseYPos = 0.0;

static double mouseSensitivity = 0.5;

static int windowWidth = 1920;
static int windowHeight = 1080;

static bool didMove = false;

static bool shouldRecalculate = false;

int main() {
	Global::Initialize();

	Console::Assert(glfwInit(), "Failed GLFW Initialization!");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	//////////////////////////////////////////////////////////////////////////////////////////////
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Procedural Generation", NULL, NULL);
	Input* input = new Input();
	input->SetShouldCaptureKeyboardInput(true);
	input->SetShouldCaptureMouseInput(true);
	Console::Assert(window, "Failed Window Creation!");
	glfwMakeContextCurrent(window);
	glfwSetWindowUserPointer(window, input);
	//////////////////////////////////////////////////////////////////////////////////////////////
	GLenum err = glewInit();
	Console::Assert(err == GLEW_OK, "Failed GLEW Initialization - %s", reinterpret_cast<char const*>(glewGetErrorString(err)));
	//////////////////////////////////////////////////////////////////////////////////////////////
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_CCW);
	//////////////////////////////////////////////////////////////////////////////////////////////
	if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, 1);
	}
	glfwSwapInterval(0);
	glDebugMessageCallback(GLDebugMessageCallback, 0);
	glfwSetErrorCallback([](int error, const char* description)
		{
			Console::Error("GLFW Error (%i): \"%s\"", error, description);
		}
	);
	//////////////////////////////////////////////////////////////////////////////////////////////
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(windowWidth, windowHeight);
	static auto font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\tahoma.ttf", 14);
	io.IniFilename = nullptr;
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	io.KeyMap[ImGuiKey_Tab] = AD_KEY_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = AD_KEY_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = AD_KEY_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = AD_KEY_UP;
	io.KeyMap[ImGuiKey_DownArrow] = AD_KEY_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = AD_KEY_PAGE_UP;
	io.KeyMap[ImGuiKey_PageDown] = AD_KEY_PAGE_DOWN;
	io.KeyMap[ImGuiKey_Home] = AD_KEY_HOME;
	io.KeyMap[ImGuiKey_End] = AD_KEY_END;
	io.KeyMap[ImGuiKey_Insert] = AD_KEY_INSERT;
	io.KeyMap[ImGuiKey_Delete] = AD_KEY_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = AD_KEY_BACKSPACE;
	io.KeyMap[ImGuiKey_Space] = AD_KEY_SPACE;
	io.KeyMap[ImGuiKey_Enter] = AD_KEY_ENTER;
	io.KeyMap[ImGuiKey_Escape] = AD_KEY_ESCAPE;
	io.KeyMap[ImGuiKey_KeyPadEnter] = AD_KEY_KP_ENTER;
	io.KeyMap[ImGuiKey_A] = AD_KEY_A;
	io.KeyMap[ImGuiKey_C] = AD_KEY_C;
	io.KeyMap[ImGuiKey_V] = AD_KEY_V;
	io.KeyMap[ImGuiKey_X] = AD_KEY_X;
	io.KeyMap[ImGuiKey_Y] = AD_KEY_Y;
	io.KeyMap[ImGuiKey_Z] = AD_KEY_Z;
	ImGui_ImplOpenGL3_Init("#version 460");
	//////////////////////////////////////////////////////////////////////////////////////////////
	glfwSetCursorPosCallback(window, [](GLFWwindow* glfwWindow, double xPos, double yPos)
		{
			Input* input = (Input*)glfwGetWindowUserPointer(glfwWindow);

			input->MoveMouseTo(xPos, yPos);

			mouseXPos = xPos;
			mouseYPos = yPos;

			ImGuiIO& io = ImGui::GetIO();
			io.MousePos = ImVec2(xPos, yPos);

			didMove = true;
		}
	);
	glfwSetMouseButtonCallback(window, [](GLFWwindow* glfwWindow, int button, int action, int mods)
		{
			Input* input = (Input*)glfwGetWindowUserPointer(glfwWindow);

			ImGuiIO& io = ImGui::GetIO();

			switch (action)
			{
			case GLFW_PRESS:
			{
				input->UpdateMouseButton(button, 1);
				if (io.WantCaptureMouse) {
					io.MouseDown[button] = true;
				}
				break;
			}
			case GLFW_RELEASE:
			{
				input->UpdateMouseButton(button, 3);
				if (io.WantCaptureMouse) {
					io.MouseDown[button] = false;
				}
				break;
			}
			}
		}
	);
	glfwSetKeyCallback(window, [](GLFWwindow* glfwWindow, int key, int scancode, int action, int mods)
		{
			Input* input = (Input*)glfwGetWindowUserPointer(glfwWindow);

			ImGuiIO& io = ImGui::GetIO();

			switch (action)
			{
			case GLFW_PRESS:
			{
				input->UpdateKeyboardKey(key, 1);
				if (io.WantCaptureKeyboard) {
					io.KeysDown[key] = true;

					io.KeyCtrl = io.KeysDown[AD_KEY_LEFT_CONTROL] || io.KeysDown[AD_KEY_RIGHT_CONTROL];
					io.KeyShift = io.KeysDown[AD_KEY_LEFT_SHIFT] || io.KeysDown[AD_KEY_RIGHT_SHIFT];
					io.KeyAlt = io.KeysDown[AD_KEY_LEFT_ALT] || io.KeysDown[AD_KEY_RIGHT_ALT];
					io.KeySuper = io.KeysDown[AD_KEY_LEFT_SUPER] || io.KeysDown[AD_KEY_RIGHT_SUPER];
				}
				break;
			}
			case GLFW_RELEASE:
			{
				input->UpdateKeyboardKey(key, 3);
				if (io.WantCaptureKeyboard) {
					io.KeysDown[key] = false;
				}
				break;
			}
			}
		}
	);
	glfwSetWindowSizeCallback(window, [](GLFWwindow* glfwWindow, int width, int height)
		{
			Input* input = (Input*)glfwGetWindowUserPointer(glfwWindow);

			windowWidth = width;
			windowHeight = height;

			ImGuiIO& io = ImGui::GetIO();

			io.DisplaySize = ImVec2(windowWidth, windowHeight);
			io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		}
	);
	glfwSetCharCallback(window, [](GLFWwindow* glfwWindow, unsigned int keycode)
		{
			Input* input = (Input*)glfwGetWindowUserPointer(glfwWindow);

			ImGuiIO& io = ImGui::GetIO();

			if (io.WantCaptureKeyboard) {
				if (keycode > 0 && keycode < 0x10000) {
					io.AddInputCharacter((unsigned short)keycode);
				}
			}
		}
	);
	glfwSetScrollCallback(window, [](GLFWwindow* glfwWindow, double xOffset, double yOffset)
		{
			Input* input = (Input*)glfwGetWindowUserPointer(glfwWindow);

			ImGuiIO& io = ImGui::GetIO();
			if (io.WantCaptureMouse) {
				io.MouseWheelH += xOffset;
				io.MouseWheel += yOffset;
			}
		}
	);
	glfwSetCursorEnterCallback(window, [](GLFWwindow* glfwWindow, int entered)
		{
			Input* input = (Input*)glfwGetWindowUserPointer(glfwWindow);

			input->SetMouseWasBlocked(true);
		}
	);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);












	//Texture tex = Texture("res/textures/basic.png");
	Texture tex = Texture(TEXTURE_SIZE, TEXTURE_SIZE);
	glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);
	glm::mat4 transformation = glm::translate(glm::mat4(1.0f), glm::vec3(1870.0f, 1030.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 1.0f));
	Shader shader = Shader("res/shaders/2d", SHADER_FRAGMENT_SHADER | SHADER_VERTEX_SHADER);
	GLfloat vertices[] = {
		-0.5f, -0.5f, // Bottom Left
		0.0f, 0.0f,   // 
		-0.5f, 0.5f,  // Top Left
		0.0f, 1.0f,   // 
		0.5f, 0.5f,   // Top Right
		1.0f, 1.0f,   // 

		-0.5f, -0.5f, // Bottom Left
		0.0f, 0.0f,   // 
		0.5f, 0.5f,   // Top Right
		1.0f, 1.0f,   // 
		0.5f, -0.5f,  // Bottom Right
		1.0f, 0.0f    // 
	};
	//2d
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	GLuint VAO, VBO;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(sizeof(GLfloat) * 2));












	//3d
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	Mesh3D environment = Mesh3D("res/models/environment.obj");
	glm::vec3 environmentTranslation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 environmentRotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 environmentScale = glm::vec3(10.0f, 10.0f, 10.0f);
	glm::mat4 environmentTransformation = glm::translate(glm::mat4(1.0f), environmentTranslation) * glm::yawPitchRoll(glm::radians(environmentRotation.x), glm::radians(environmentRotation.y), glm::radians(environmentRotation.z)) * glm::scale(glm::mat4(1.0f), environmentScale);
	Shader environmentShader = Shader("res/shaders/3d", SHADER_FRAGMENT_SHADER | SHADER_VERTEX_SHADER);
	Texture environmentTexture = Texture("res/textures/White.png");
	Vertex3D* environmentVertices = environment.GetVertices();
	unsigned int numEnvironmentVertices = environment.GetNumberOfVertices();
	unsigned int currentEnvironmentVertex = 0;
	noise::module::Perlin environmentPerlinModule;
	environmentPerlinModule.SetSeed(1000);
	double currentFrequency = 0.125;
	environmentPerlinModule.SetFrequency(currentFrequency);
	for (unsigned int i = 0; i < numEnvironmentVertices; i++) {
		float xCoord = environmentVertices[i].position.x;
		float yCoord = environmentVertices[i].position.z;
		float noiseValue = environmentPerlinModule.GetValue(xCoord, yCoord, 0.0f);
		environmentVertices[i].position.y = 5.0f * noiseValue;
	}
	environment.LoadToVRAM();
	Camera camera = Camera();
	camera.SetTranslation(glm::vec3(0.0f, 50.0f, 0.0f));


	double timeConstant = 1.0;
	auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = lastTime;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	while (!glfwWindowShouldClose(window)) {
		currentTime = std::chrono::high_resolution_clock::now();
		auto deltaTimeNanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - lastTime);
		lastTime = currentTime;
		double deltaTimeWithoutTimeFactor = deltaTimeNanoseconds.count() / 1000000000.0;
		double deltaTime = deltaTimeWithoutTimeFactor * timeConstant;

		glfwPollEvents();
		input->Prepare();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//update
		if (shouldRecalculate) {
			projection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);
			glViewport(0, 0, windowWidth, windowHeight);
		}
		if (input->GetKeyboardKeyHeld(AD_KEY_W)) {
			camera.MoveForward(deltaTime);
		}
		if (input->GetKeyboardKeyHeld(AD_KEY_A)) {
			camera.StrafeLeft(deltaTime);
		}
		if (input->GetKeyboardKeyHeld(AD_KEY_S)) {
			camera.MoveBackward(deltaTime);
		}
		if (input->GetKeyboardKeyHeld(AD_KEY_D)) {
			camera.StrafeRight(deltaTime);
		}
		if (input->GetKeyboardKeyHeld(AD_KEY_LEFT_CONTROL)) {
			camera.MoveDown(deltaTime);
		}
		if (input->GetKeyboardKeyHeld(AD_KEY_SPACE)) {
			camera.MoveUp(deltaTime);
		}
		if (input->GetKeyboardKeyPressed(AD_KEY_LEFT_SHIFT)) {
			camera.SetMovementSpeed(25.0f);
		}
		if (input->GetKeyboardKeyReleased(AD_KEY_LEFT_SHIFT)) {
			camera.SetMovementSpeed(5.0f);
		}

		//if (input->GetMouseButtonPressed(AD_MOUSE_BUTTON_LEFT)) {
		//	if (currentEnvironmentVertex < numEnvironmentVertices) {
		//		float xCoord = environmentVertices[currentEnvironmentVertex].position.x;
		//		float yCoord = environmentVertices[currentEnvironmentVertex].position.z;
		//		environmentVertices[currentEnvironmentVertex].position.y = 5.0f * environmentPerlinModule.GetValue(xCoord, yCoord, 0.0f);
		//		environment.UnloadFromVRAM();
		//		environment.LoadToVRAM();
		//		currentEnvironmentVertex++;
		//	}
		//}
		if (input->GetMouseButtonPressed(AD_MOUSE_BUTTON_LEFT)) {
			currentFrequency *= 2.0;
			environmentPerlinModule.SetFrequency(currentFrequency);
			environment.UnloadFromVRAM();
			for (unsigned int i = 0; i < numEnvironmentVertices; i++) {
				float xCoord = environmentVertices[i].position.x;
				float yCoord = environmentVertices[i].position.z;
				float noiseValue = environmentPerlinModule.GetValue(xCoord, yCoord, 0.0f);
				environmentVertices[i].position.y = 5.0f * noiseValue;
			}
			environment.LoadToVRAM();
		}

		if (input->GetMouseButtonPressed(AD_MOUSE_BUTTON_RIGHT)) {
			currentFrequency /= 2.0;
			environmentPerlinModule.SetFrequency(currentFrequency);
			environment.UnloadFromVRAM();
			for (unsigned int i = 0; i < numEnvironmentVertices; i++) {
				float xCoord = environmentVertices[i].position.x;
				float yCoord = environmentVertices[i].position.z;
				float noiseValue = environmentPerlinModule.GetValue(xCoord, yCoord, 0.0f);
				environmentVertices[i].position.y = 5.0f * noiseValue;
			}
			environment.LoadToVRAM();
		}

		if (input->GetMouseButtonPressed(AD_MOUSE_BUTTON_MIDDLE)) {
			glm::vec3 temp = camera.GetForwardDirection();
			Console::Info("    Forward: (%.3f, %.3f, %.3f)", temp.x, temp.y, temp.z);
		}

		if (didMove) {
			camera.LookAtMouse(mouseSensitivity, input->GetMousePositionX(), input->GetMousePositionY(), input->GetOldMousePositionX(), input->GetOldMousePositionY());
		}

		//draw3d
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		environmentShader.Bind();
		environmentTexture.Bind(0);
		environment.Bind();
		environmentShader.SetMat4("transformation", environmentTransformation);
		environmentShader.SetMat4("view", camera.GetViewMatrix());
		environmentShader.SetMat4("projection", camera.GetProjectionMatrix(windowWidth, windowHeight));
		environment.Draw();

		//draw2d
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		shader.Bind();
		shader.SetMat4("projection", projection);
		shader.SetMat4("transformation", transformation);
		tex.Bind(0);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);

		input->Flush();
		didMove = false;
	}
}