#include "MrpocPch.h"

#include <Resources/Shader.h>
#include <Resources/Mesh.h>
#include <Resources/ResourceHandler.h>
#include <SceneManagement/Node.h>
#include <SceneManagement/Nodes/Camera.h>
#include <SceneManagement/Nodes/Light.h>
#include <SceneManagement/Nodes/Model.h>

#include <Graphics/Vertex.h>
#include <Graphics/Renderer.h>

#include "IO/GltfLoader.h"

#include "SceneManagement/Scene.h"

#include "EventSystem/EventMessenger.h"
#include "EventSystem/EventArgs.h"
#include "Window/Window.h"
#include <chrono>

using namespace mrpoc;

static const int s_clientWidth = 800;
static const int s_clientHeight = 600;

static const float s_CamMoveSpeed = 0.0015f;
static const float s_CameraRotationSpeed = 0.005f;

void InitDebugMessages();
void HandleCameraBehaviour(mrpoc::Window& window, mrpoc::Node* camNode, float deltaTime);

int main(int argc, char** argv)
{
	EventMessenger::GetInstance().AddMessenger<ResizeEventArgs&>("OnWindowResize");

	mrpoc::Window window;
	window.Initialize("Suberb Interdashing MrPoc", s_clientWidth, s_clientHeight);

	InitDebugMessages();

	std::vector<mrpoc::Scene> testScenes;
	ResourceHandler resourceHandler;
	Renderer renderer(resourceHandler);
	GLTFLoader gltfLoader;
	std::chrono::high_resolution_clock timer;
	//gltfLoader.LoadGLTFIntoScene(resourceHandler, testScenes, "resources/NormalTangentMirrorTest/glTF/NormalTangentMirrorTest.gltf");
	//gltfLoader.LoadGLTFIntoScene(resourceHandler, testScenes, "resources/NormalTangentMirrorTest/glTF/NormalTangentMirrorTest_PLights.gltf");
	//gltfLoader.LoadGLTFIntoScene(resourceHandler, testScenes, "resources/NormalTangentMirrorTest/glTF/NormalTangentMirrorTest._AngleTestgltf.gltf");
	//gltfLoader.LoadGLTFIntoScene(resourceHandler, testScenes, "resources/Cerberus/glTF/CerberusWithColoredPLights.gltf", true);
	//gltfLoader.LoadGLTFIntoScene(resourceHandler, testScenes, "resources/Cerberus/glTF/Cerberus.gltf", true);
	//gltfLoader.LoadGLTFIntoScene(resourceHandler, testScenes, "resources/Bistro/bistroInterior.gltf", true);
	//gltfLoader.LoadGLTFIntoScene(resourceHandler, testScenes, "resources/Bistro/bistroInteriorOnlyLights.gltf", true);
	//gltfLoader.LoadGLTFIntoScene(resourceHandler, testScenes, "resources/TD_AssetPack/Map_1/SceneRoot.gltf", true);
	gltfLoader.LoadGLTFIntoScene(resourceHandler, testScenes, "resources/TD_AssetPack/Map_1/TD_MapWithManualMats.gltf", true);
	//gltfLoader.LoadGLTFIntoScene(resourceHandler, testScenes, "resources/SponzaWithLights/sponzaWithColoredLights.gltf", true);
	//gltfLoader.LoadGLTFIntoScene(resourceHandler, testScenes, "resources/TD_AssetPack/EnemyShip_1/Enemy_Gatling.gltf", true);
	//gltfLoader.LoadGLTFIntoScene(resourceHandler, testScenes, "resources/Totem/gltf/Totem.gltf", true);
	//gltfLoader.LoadGLTFIntoScene(resourceHandler, testScenes, "resources/Test/Australopitecus.gltf", true);

	
	float deltaTime = 0;
	while (!window.CheckWindowCloseRequest())
	{
		auto timeAtStartOfFrame = timer.now();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if (window.GetKeyState(GLFW_KEY_ESCAPE))
		{
			break;
		}
		HandleCameraBehaviour(window, testScenes[0].GetRoot()->GetActiveCamera(renderer.GetActiveCameraIndex()), deltaTime);

		// new not finished work
		{
			testScenes[0].Update();
			renderer.Render(testScenes[0]);
		}

		window.SwapBuffers();
		window.PollWindowEvents();

		auto timeAtEndOfFrame = timer.now();
		deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(timeAtEndOfFrame - timeAtStartOfFrame).count()/1000.f;
	}

	window.Destroy();
	glfwTerminate();
	return 0;
}

void HandleCameraBehaviour(mrpoc::Window& window, mrpoc::Node* camNode, float deltaTime)
{
	//spdlog::info("Current Z forward of camera: X:"+ std::to_string(camNode->GetWorldTransform()[2].x) + " Y:" + std::to_string(camNode->GetWorldTransform()[2].y) + " Z:" + std::to_string(camNode->GetWorldTransform()[2].z));
	glm::mat3 movementMatrix = glm::inverse(camNode->GetLocalTransform());
	glm::vec3 UpVector = glm::vec3(0,1,0) * movementMatrix;
	glm::vec3 RightVector = glm::vec3(1,0,0) * movementMatrix;
	glm::vec3 ForwardVector = glm::vec3(0,0,1) * movementMatrix;

	/*glm::mat3 movementMatrix = camNode->GetWorldTransform();
	glm::vec3 UpVector = glm::vec3(movementMatrix[1].x, movementMatrix[1].y, movementMatrix[1].z);
	glm::vec3 RightVector = glm::vec3(movementMatrix[0].x, movementMatrix[0].y, movementMatrix[0].z);
	glm::vec3 ForwardVector = glm::vec3(movementMatrix[2].x, movementMatrix[2].y, movementMatrix[2].z);*/

	if(window.GetKeyState(GLFW_KEY_SPACE))
	{
		camNode->Translate(s_CamMoveSpeed * UpVector * deltaTime);
	}
	else if(window.GetKeyState(GLFW_KEY_LEFT_SHIFT))
	{
		camNode->Translate(-s_CamMoveSpeed * UpVector * deltaTime);
	}

	if(window.GetKeyState(GLFW_KEY_A))
	{
		camNode->Translate(-s_CamMoveSpeed * RightVector * deltaTime);
	}
	else if(window.GetKeyState(GLFW_KEY_D))
	{
		camNode->Translate(s_CamMoveSpeed * RightVector * deltaTime);
	}

	if(window.GetKeyState(GLFW_KEY_Q))
	{
		camNode->Rotate(s_CameraRotationSpeed * glm::vec3(0,0,1) * deltaTime);
	}
	else if(window.GetKeyState(GLFW_KEY_E))
	{
		camNode->Rotate(-s_CameraRotationSpeed * glm::vec3(0, 0, 1) * deltaTime);
	}

	if (window.GetKeyState(GLFW_KEY_W))
	{
		camNode->Translate(-s_CamMoveSpeed * ForwardVector * deltaTime);
	}
	else if (window.GetKeyState(GLFW_KEY_S))
	{
		camNode->Translate(s_CamMoveSpeed * ForwardVector * deltaTime);
	}

	if (window.GetKeyState(GLFW_MOUSE_BUTTON_LEFT)) // check if need to strafe
	{
		camNode->Translate((static_cast<float>(window.GetDeltaMouseXPos()) * -s_CamMoveSpeed * RightVector * deltaTime) + (static_cast<float>(window.GetDeltaMouseYPos()) * s_CamMoveSpeed * UpVector * deltaTime));
	}

	if (window.GetKeyState(GLFW_MOUSE_BUTTON_RIGHT))
	{
		camNode->Rotate((static_cast<float>(window.GetDeltaMouseXPos()) * deltaTime * s_CameraRotationSpeed * glm::vec3(0, 1, 0)));
		camNode->Rotate((static_cast<float>(window.GetDeltaMouseYPos()) * deltaTime * s_CameraRotationSpeed * glm::vec3(1, 0, 0)));
	}
	window.ResetDeltaMousePositions();
}

static void APIENTRY DebugCallbackFunc(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam)
{
	// Skip some less useful info
	switch (id)
	{
	case 131218:	// http://stackoverflow.com/questions/12004396/opengl-debug-context-performance-warning
		return;
	}

	std::string sourceString;
	std::string typeString;
	std::string severityString;

	switch (source) {
	case GL_DEBUG_SOURCE_API: {
		sourceString = "API";
		break;
	}
	case GL_DEBUG_SOURCE_APPLICATION: {
		sourceString = "Application";
		break;
	}
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
		sourceString = "Window System";
		break;
	}
	case GL_DEBUG_SOURCE_SHADER_COMPILER: {
		sourceString = "Shader Compiler";
		break;
	}
	case GL_DEBUG_SOURCE_THIRD_PARTY: {
		sourceString = "Third Party";
		break;
	}
	case GL_DEBUG_SOURCE_OTHER: {
		sourceString = "Other";
		break;
	}
	default: {
		sourceString = "Unknown";
		break;
	}
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR: {
		typeString = "Error";
		break;
	}
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
		typeString = "Deprecated Behavior";
		break;
	}
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
		typeString = "Undefined Behavior";
		break;
	}
	case GL_DEBUG_TYPE_PERFORMANCE: {
		typeString = "Performance";
		break;
	}
	case GL_DEBUG_TYPE_OTHER: {
		typeString = "Other";
		break;
	}
	default: {
		typeString = "Unknown";
		break;
	}
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: {
		severityString = "High";
		break;
	}
	case GL_DEBUG_SEVERITY_MEDIUM: {
		severityString = "Medium";
		break;
	}
	case GL_DEBUG_SEVERITY_LOW: {
		severityString = "Low";
		break;
	}
	default: {
		severityString = "Unknown";
		return;
	}
	}

	spdlog::error("GL Debug Callback:");
	spdlog::error("  source:       {source}:{sourceString}", fmt::arg("source", source), fmt::arg("sourceString", sourceString.c_str()));
	spdlog::error("  type:         {source}:{typeString}", fmt::arg("source", type), fmt::arg("typeString", typeString.c_str()));
	spdlog::error("  id:           {id}", fmt::arg("id", id));
	spdlog::error("  severity:     {severity}:{severityString}", fmt::arg("severity", severity), fmt::arg("severityString", severityString.c_str()));
	spdlog::error("  message:      {message}", fmt::arg("message", message));
	spdlog::error("------------------------------------------------------------------");
	// ASSERT(type != GL_DEBUG_TYPE_ERROR, "GL Error occurs.");
}

void InitDebugMessages()
{
	// Query the OpenGL function to register your callback function.
	PFNGLDEBUGMESSAGECALLBACKPROC	 _glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)wglGetProcAddress("glDebugMessageCallback");

	glDebugMessageCallback(DebugCallbackFunc, nullptr);

	// Register your callback function.
	if (_glDebugMessageCallback != nullptr)
	{
		_glDebugMessageCallback(DebugCallbackFunc, nullptr);
	}

	// Enable synchronous callback. This ensures that your callback function is called
	// right after an error has occurred. This capability is not defined in the AMD
	// version.
	if (_glDebugMessageCallback != nullptr)
	{
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	glDebugMessageControl(
		GL_DONT_CARE,
		GL_DONT_CARE,
		GL_DEBUG_SEVERITY_LOW,
		0,
		nullptr,
		GL_FALSE
	);

	glDebugMessageControl(
		GL_DONT_CARE,
		GL_DONT_CARE,
		GL_DEBUG_SEVERITY_NOTIFICATION,
		0,
		nullptr,
		GL_FALSE
	);
}