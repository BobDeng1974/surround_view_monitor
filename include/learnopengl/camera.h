#ifndef CAMERA_H
#define CAMERA_H

#ifndef PI
#define PI 3.14159
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};//相机移动（前后左右四个方向）

// Default camera values
const float YAW         = -90.0f;//偏航角（左右）
const float PITCH       =  0.0f;//俯仰角（上下）
const float SPEED       =  2.5f;//速度
const float SENSITIVITY =  0.1f;//灵敏度changed0.1
const float ZOOM        =  45.0f;//放大


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;//方向向量：向前
    glm::vec3 Up;//方向向量：向上
    glm::vec3 Right;//方向向量：向右
    glm::vec3 WorldUp;
    // Euler Angles
    float Yaw;//偏航角（左右）
    float Pitch;//俯仰角（上下）
    // Camera options
    float MovementSpeed;//移动速度
    float MouseSensitivity;//鼠标灵敏度
    float Zoom;//放大

    // Constructor with vectors（position, up, yaw, pitch）
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // Constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;//changed
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // Update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

	void move_camera_in_sphere(float xoffset, float yoffset, glm::vec3 target_position)
	{
		xoffset *= MouseSensitivity * 0.1f;
		yoffset *= MouseSensitivity * 0.1f;

		Yaw   -= xoffset;
		Pitch -= yoffset;

		float alpha = degree_to_radian(Yaw);
		float beta  = degree_to_radian(Pitch);

		float distance = distance_from_camera_to_target(Position, target_position);

		Position.x = target_position.x - distance * cos(alpha) * cos(beta);
		Position.y = target_position.y - distance * sin(beta); 
		Position.z = target_position.z - distance * sin(alpha) * cos(beta);

		updateCameraVectors();
	}

	void move_camera_in_ellipsoid(float xoffset, float yoffset, glm::vec3 target_position)
	{
		/*
			椭球长：宽：高 = 3：2：2 = x : z : y
			distance = 宽
		*/
		xoffset *= MouseSensitivity * 0.1f;
		yoffset *= MouseSensitivity * 0.1f;

		Yaw -= xoffset;
		Pitch -= yoffset;

		float alpha = degree_to_radian(Yaw);
		float beta = degree_to_radian(Pitch);

		float distance = distance_from_camera_to_target(Position, target_position);

		Position.x = target_position.x - distance * cos(alpha) * cos(beta);
		Position.y = target_position.y - distance * sin(beta);
		Position.z = target_position.z - distance * sin(alpha) * cos(beta);

		updateCameraVectors();
	}

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        if (Zoom >= 1.0f && Zoom <= 45.0f)
            Zoom -= yoffset;
        if (Zoom <= 1.0f)
            Zoom = 1.0f;
        if (Zoom >= 45.0f)
            Zoom = 45.0f;
    }

	// Print Camera Information
	void printInfo()
	{
		std::cout << "Position: " << Position.x << ", " << Position.y << ", " << Position.z << std::endl;
		std::cout << "WorldUp: " << WorldUp.x << ", " << WorldUp.y << ", " << WorldUp.z << std::endl;
		std::cout << "Yaw: " << Yaw << std::endl;
		std::cout << "Pitch: " << Pitch << std::endl;
	}

	
private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // Also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = glm::normalize(glm::cross(Right, Front));
    }

	float distance_from_camera_to_target(glm::vec3 camera_position, glm::vec3 target_position)
	{
		float distance_x = camera_position.x - target_position.x;
		float distance_y = camera_position.y - target_position.y;
		float distance_z = camera_position.z - target_position.z;

		return sqrt(distance_x * distance_x + distance_y * distance_y + distance_z * distance_z);
	}

	float degree_to_radian(float degree)
	{
		return degree / 180.0f * PI;
	}
};
#endif