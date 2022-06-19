#pragma once

#include <glm/glm.hpp>

/*
* Camera object
*/
struct Camera {
	//Location of camera (World Coordinates)
	glm::vec3 loc;

	//Location camera is looking at (World Coordinates)
	glm::vec3 lookAt;

	//Horizontal Field of View (1.0f = 90 degrees)
	float fov;
};