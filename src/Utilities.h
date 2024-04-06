#ifndef UTILITIES_CLASS_H
#define UTILITIES_CLASS_H

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080


#include "Model.h"
#include "Shader.h"

struct Camera {
	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 direction;
	glm::vec3 up;
	glm::vec3 front;
	glm::mat4 view;
	glm::mat4 projection;
	float fov;
	float pitch;
	float yaw;

	Camera()
		: position(glm::vec3(0.0f, 0.0f, 10.0f)),
		target(glm::vec3(0.0f, 0.0f, 0.0f)),
		up(glm::vec3(0.0f, 1.0f, 0.0f)),
		front(glm::vec3(0.0f, 0.0f, -1.0f)),
		view(glm::mat4(1.0f)),
		yaw(-90.0f),
		pitch(0.0f),
		fov(45.0f) {
		direction = glm::normalize(position - target);
		projection = glm::perspective(fov, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f);
	}
};

struct Mouse {
	bool first;
	float xPrevious, yPrevious;
	float x, y;
	float sensitivity;
	Mouse()
		: first(true),
		xPrevious(800.0f / 2.0),
		yPrevious(800.0f / 2.0),
		x(0.f), y(0.f), sensitivity(100.0f)
	{}
};



#endif // !UTILITIES_CLASS_H
