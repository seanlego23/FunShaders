#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>

#include "mandelbowl.h"
#include "mandelbrot.h"
#include "screen.h"
#include "shader.h"
#include "shader_inputs.h"
#include "shader_object.h"

constexpr auto PAN_BUTTON_MASK = 0x1;
constexpr auto ROTATE_BUTTON_MASK = 0x2;

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

static shader_object* curobj;
static screen* curscr;
static int button_mask = 0;

static float rotate_speed = 350.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

bool test_arcball(float x_angle, float y_angle, const glm::vec3 loc, const glm::vec3 up, const glm::vec3 right);

void drawImGui() {

	float zoom = curobj->get_inputs()->zoom;
	glm::vec3 loc = curscr->camera.loc;
	float fov = curscr->camera.fov;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Shaders");

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	bool changeZoom = ImGui::DragFloat("Zoom", &zoom, 0.25f, 1.0f, 10000.0f);
	bool changeLoc = ImGui::DragFloat3("Location", &loc.x, 0.01f, -10.0f, 10.0f);
	bool changeFOV = ImGui::DragFloat("FOV", &fov, 0.01f, 0.1f, 1.75f);

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (changeZoom) {
		curobj->get_inputs()->zoom = zoom;
		curobj->get_inputs()->zoomRaw = glm::log(zoom);
	}

	if (changeLoc) {
		curscr->camera.loc = loc;
	}

	if (changeFOV) {
		curscr->camera.fov = fov;
	}
}

void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // ignore these non-significant error codes

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

int main(int argc, const char* argv[]) {
	//Initialize glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	//Create window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Digital Notes", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD\n";
		glfwTerminate();
		return -1;
	}

	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	namespace fs = std::filesystem;
	
	fs::path data(*argv);
	data.remove_filename();
	fs::current_path(data);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	const char* glsl_version = "#version 460";
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	if (!shader::init_vert())
		return -1;

	mandelbrot mandel({ 0.0f, 0.8f, glm::log(0.8f) });

	mandelbowl bowl;

	curobj = &bowl;

	screen scr;
	scr.setResolution({SCR_WIDTH, SCR_HEIGHT});
	scr.camera.loc = glm::vec3(0.0f, -2.0f, 1.0);
	scr.camera.dir = glm::normalize(glm::vec3(0.0f) - scr.camera.loc);
	scr.camera.right = glm::vec3(1.0f, 0.0f, 0.0f);
	scr.camera.up = glm::normalize(glm::cross(scr.camera.right, scr.camera.dir));
	scr.camera.fov = 1.0;

	curscr = &scr;

	double time = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	double elapsedTime = 0.0;

	while (!glfwWindowShouldClose(window)) {

		elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() - time;
		time += elapsedTime;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		curobj->get_inputs()->elapsedTime = (float)elapsedTime;

		curscr->draw_screen(curobj);

		drawImGui();

		glfwSwapBuffers(window);

		glfwPollEvents();

	}

	shader::destroy_vert();

	//Terminate glfw
	glfwTerminate();
}

glm::vec2 screenToWorld(glm::vec2 coord) {
	glm::vec2 res = curscr->getResolution();
	return (2.0f * coord - res) / (res.y * curobj->get_inputs()->zoom) + glm::vec2(curscr->camera.loc);
}

glm::vec2 screenToWorldDir(glm::vec2 dir) {
	glm::vec2 res = curscr->getResolution();
	return 2.0f * dir / (res.y * curobj->get_inputs()->zoom);
}

glm::vec2 worldToScreen(glm::vec2 coord) {
	glm::vec2 res = curscr->getResolution();
	return ((coord - glm::vec2(curscr->camera.loc)) * (res.y * curobj->get_inputs()->zoom) + res) / 2.0f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	curscr->setResolution({width, height});
	curobj->framebuffer_resize(width, height);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	glm::vec2 res = curscr->getResolution();
	//Flip the y because rendering is done from lower left and cursor is from upper left
	glm::vec2 new_pos = glm::vec2((float)xpos, res.y - (float)ypos);

	glm::vec2 old_pos = curscr->getCursorPos();
	glm::vec2 s_dir = screenToWorldDir(new_pos - old_pos);

	if (button_mask & PAN_BUTTON_MASK) {
		//Pan the camera in reference to current camera plane
		glm::vec3 x_dir = curscr->camera.right * s_dir.x;
		glm::vec3 y_dir = curscr->camera.up * s_dir.y;
		glm::vec3 n_dir = x_dir + y_dir;
		curscr->camera.loc -= n_dir;
	} else if (button_mask & ROTATE_BUTTON_MASK) {
		//Arcball rotation around camera's lookAt point
		float x_scale = -s_dir.x / res.x * rotate_speed * 2.0f;
		float y_scale = s_dir.y / res.y * rotate_speed;
		float x_sin_half = glm::sin(x_scale / 2); //Half angle for quaternion
		float y_sin_half = glm::sin(y_scale / 2);
		float x_cos_half = glm::cos(x_scale / 2);
		float y_cos_half = glm::cos(y_scale / 2);
		glm::vec3 x_rot = curscr->camera.up * x_sin_half;
		glm::vec3 y_rot = curscr->camera.right * y_sin_half;
		glm::quat x_quat(x_cos_half, x_rot);
		glm::quat y_quat(y_cos_half, y_rot);
		glm::quat qua = x_quat * y_quat;
		glm::mat4 rot_mat(qua);
		
		glm::vec3 cam_dir = curscr->camera.dir;
		glm::vec3 ball_point = curscr->camera.loc + cam_dir;
		curscr->camera.loc = ball_point - glm::vec3(rot_mat * glm::vec4(cam_dir, 0.0f));
		curscr->camera.up = glm::vec3(rot_mat * glm::vec4(curscr->camera.up, 0.0f));
		curscr->camera.right = glm::vec3(rot_mat * glm::vec4(curscr->camera.right, 0.0f));
		curscr->camera.dir = glm::normalize(glm::cross(curscr->camera.up, curscr->camera.right));
	}

	curscr->setCursorPos(new_pos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	shader_inputs* curin = curobj->get_inputs();
	
	curin->zoomRaw += (float)yoffset;

	/*glm::vec2 curpos = curscr->getCursorPos();
	glm::vec2 pos = screenToWorld(curpos);

	shader_inputs* curin = curobj->get_inputs();

	curin->zoomRaw += (float)yoffset;
	curin->zoom = glm::exp(0.05f * curin->zoomRaw);

	glm::vec2 new_cursorPos = worldToScreen(pos);
	curscr->camera.loc += glm::vec3(screenToWorldDir(new_cursorPos - curpos), 0.0f)*/;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		if (action == GLFW_PRESS)
			button_mask = !button_mask ? PAN_BUTTON_MASK : button_mask;
		else 
			button_mask = button_mask & PAN_BUTTON_MASK ? 0 : button_mask;
	} else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS)
			button_mask = !button_mask ? ROTATE_BUTTON_MASK : button_mask;
		else
			button_mask = button_mask & ROTATE_BUTTON_MASK ? 0 : button_mask;
	}
}

std::ostream& operator<<(std::ostream& out, const glm::vec3& obj) {
	out << "(" << obj.x << ", " << obj.y << ", " << obj.z << ")";
	return out;
}

bool equalf(float a, float b) {
	constexpr float FLOAT_PREC = 0.0000005f;
	float diff = std::abs(b - a);
	return diff < FLOAT_PREC || diff < std::abs(a * FLOAT_PREC) || diff < std::abs(b * FLOAT_PREC);
}

bool equal(const glm::vec3& v1, const glm::vec3& v2) {
	return equalf(v1.x, v2.x) && equalf(v1.y, v2.y) && equalf(v1.z, v2.z);
}

bool test_arcball(float x_angle, float y_angle, const glm::vec3 loc, const glm::vec3 up, const glm::vec3 right) {
	float x_sin_half = glm::sin(x_angle / 2); //Half angle for quaternion
	float y_sin_half = glm::sin(y_angle / 2);
	float x_cos_half = glm::cos(x_angle / 2);
	float y_cos_half = glm::cos(y_angle / 2);
	glm::vec3 x_rot = curscr->camera.up * x_sin_half;
	glm::vec3 y_rot = curscr->camera.right * y_sin_half;
	glm::quat x_quat(x_cos_half, x_rot);
	glm::quat y_quat(y_cos_half, y_rot);
	glm::quat qua = x_quat * y_quat;
	glm::mat4 rot_mat(qua);

	glm::vec3 cam_dir = curscr->camera.dir;
	glm::vec3 ball_point = curscr->camera.loc + cam_dir;
	glm::vec3 test_loc = ball_point - glm::vec3(rot_mat * glm::vec4(cam_dir, 0.0f));
	glm::vec3 test_up = glm::vec3(rot_mat * glm::vec4(curscr->camera.up, 0.0f));
	glm::vec3 test_right = glm::vec3(rot_mat * glm::vec4(curscr->camera.right, 0.0f));

	bool bloc = equal(loc, test_loc);
	bool bup = equal(up, test_up);
	bool bright = equal(right, test_right);

	if (!bloc)
		std::cout << "Camera Location is incorrect: " << test_loc << " Correct: " << loc << "\n";
	if (!bup)
		std::cout << "Camera Up Direction is incorrect: " << test_up << " Correct: " << up << "\n";
	if (!bright)
		std::cout << "Camera Right Direction is incorrect: " << test_right << " Correct: " << right << "\n";

	return bloc && bup && bright;
}
