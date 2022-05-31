#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>

#include "screen.h"
#include "shader.h"
#include "shader_inputs.h"

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
unsigned int scr_width = SCR_WIDTH;
unsigned int scr_height = SCR_HEIGHT;

static shader_inputs* current_inputs;
static bool pan_button = false;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

void drawImGui() {

	float zoom = current_inputs->zoom;
	glm::vec2 trans = current_inputs->translation;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Shaders");

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	bool changeZoom = ImGui::DragFloat("Zoom", &zoom, 1.0f, 1.0f, 50000.0f);
	ImGui::DragFloat2("Translation", &trans.x, 0.01f, -2.0f, 2.0f);

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (changeZoom)
		current_inputs->zoomSum = glm::log(zoom);
	current_inputs->zoom = zoom;
	current_inputs->translation = trans;

}

int main(int argc, const char* argv[]) {
	//Initialize glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create window
	GLFWwindow* window = glfwCreateWindow(scr_width, scr_height, "Digital Notes", NULL, NULL);
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

	shader mandelbrot("data/mandelbrot.glsl");
	shader_inputs mandelInputs;

	mandelInputs.resolution = glm::vec2(scr_width, scr_height);
	mandelInputs.translation = glm::vec2(0.0f);
	mandelInputs.scale = glm::vec2(4.0f, 2.5f);
	mandelInputs.offset = glm::vec2(-2.0f, -1.25f);
	mandelInputs.zoom = 1.0f;
	mandelInputs.zoomSum = 0.0f;

	current_inputs = &mandelInputs;

	screen scr;

	double time = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	double elapsedTime = 0.0;

	while (!glfwWindowShouldClose(window)) {

		elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() - time;
		time += elapsedTime;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		current_inputs->elapsedTime = (float)elapsedTime;
		current_inputs->resolution = glm::vec2(scr_width, scr_height);

		scr.draw_screen(&mandelbrot, current_inputs);

		drawImGui();

		glfwSwapBuffers(window);

		glfwPollEvents();

	}

	shader::destroy_vert();

	//Terminate glfw
	glfwTerminate();
}

glm::vec2 screenToWorld(glm::vec2 coord) {
	return (coord / current_inputs->resolution * current_inputs->scale) /
		current_inputs->zoom + current_inputs->offset + current_inputs->translation;
}

glm::vec2 screenToWorldDir(glm::vec2 dir) {
	return (dir / current_inputs->resolution * current_inputs->scale) / current_inputs->zoom;
}

glm::vec2 worldToScreen(glm::vec2 coord) {
	return ((coord - current_inputs->translation - current_inputs->offset) * current_inputs->zoom) /
		current_inputs->scale * current_inputs->resolution;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, scr_width = width, scr_height = height);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	glm::vec2 new_pos = glm::vec2((float)xpos, scr_height - (float)ypos);

	if (pan_button) {
		glm::vec2 old_pos = current_inputs->cursorPos;
		current_inputs->translation -= screenToWorldDir(new_pos - old_pos);
	}

	//Flip the y because rendering is done from lower left and cursor is from upper left
	current_inputs->cursorPos = new_pos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	glm::vec2 pos = screenToWorld(current_inputs->cursorPos);
	glm::vec2 trans = current_inputs->translation;

	current_inputs->zoomSum += (float)yoffset;
	current_inputs->zoom = glm::exp(0.05f * current_inputs->zoomSum);

	glm::vec2 new_cursorPos = worldToScreen(pos);
	current_inputs->translation += screenToWorldDir(new_cursorPos - current_inputs->cursorPos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

	if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
		pan_button = action == GLFW_PRESS;
	}
 
}