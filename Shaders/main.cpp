#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void drawImGui() {

	float zoom = curobj->inputs.zoom;
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
		curobj->inputs.zoom = zoom;
		curobj->inputs.zoomRaw = glm::log(zoom);
	}

	if (changeLoc) {
		curscr->camera.loc = loc;
	}

	if (changeFOV) {
		curscr->camera.fov = fov;
	}
}

int main(int argc, const char* argv[]) {
	//Initialize glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

	mandelbrot mandel;

	mandel.inputs.zoom = 0.25f;
	mandel.inputs.zoomRaw = glm::log(mandel.inputs.zoom);

	curobj = &mandel;

	screen scr;
	scr.camera.loc = glm::vec3(0.0f, 0.0f, 2.0f);
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

		curobj->inputs.elapsedTime = (float)elapsedTime;

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
	return (2.0f * coord - res) / (res.y * curobj->inputs.zoom) + glm::vec2(curscr->camera.loc);
}

glm::vec2 screenToWorldDir(glm::vec2 dir) {
	glm::vec2 res = curscr->getResolution();
	return 2.0f * dir / (res.y * curobj->inputs.zoom);
}

glm::vec2 worldToScreen(glm::vec2 coord) {
	glm::vec2 res = curscr->getResolution();
	return ((coord - glm::vec2(curscr->camera.loc)) * (res.y * curobj->inputs.zoom) + res) / 2.0f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	curscr->setResolution({width, height});
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	glm::vec2 res = curscr->getResolution();
	//Flip the y because rendering is done from lower left and cursor is from upper left
	glm::vec2 new_pos = glm::vec2((float)xpos, res.y - (float)ypos);

	if (button_mask & PAN_BUTTON_MASK) {
		glm::vec2 old_pos = curscr->getCursorPos();
		curscr->camera.loc -= glm::vec3(screenToWorldDir(new_pos - old_pos), 0.0f);
	}

	curscr->setCursorPos(new_pos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	glm::vec2 curpos = curscr->getCursorPos();
	glm::vec2 pos = screenToWorld(curpos);

	curobj->inputs.zoomRaw += (float)yoffset;
	curobj->inputs.zoom = glm::exp(0.05f * curobj->inputs.zoomRaw);

	glm::vec2 new_cursorPos = worldToScreen(pos);
	curscr->camera.loc += glm::vec3(screenToWorldDir(new_cursorPos - curpos), 0.0f);
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