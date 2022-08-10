#pragma once

#include <glm/glm.hpp>

/*
* Camera object
*/
struct Camera {
	//Location of camera (World Coordinates)
	glm::vec3 loc;

	//Direction camera is looking
	glm::vec3 dir;

	//Up vector for camera
	glm::vec3 up;

	//Right vector for camera
	glm::vec3 right;

	//Horizontal Field of View (1.0f = 90 degrees)
	float fov;
};