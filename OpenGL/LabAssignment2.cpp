//
//  LabAssignment2.cpp
//  OpenGL
//
//  Created by Mx on 24/01/2017.
//  Copyright © 2017 Mx. All rights reserved.
//

#include <string>
#include <algorithm>
//using namespace std;
#include <iostream>

#include <cstdio>
#include <cstdlib>

//GLEW
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// GL includes
#include "libs/Shader.h"
#include "libs/Mesh.h"
#include "libs/Model.h"
#include "libs/Camera.h"

// GLM Mathemtics
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Glm
#include <glm.hpp>
#include <glm.cpp>

// Soil
#include <SOIL.h>

// Window dimensions
int screenWidth = 800, screenHeight = 600;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

GLfloat yaw = 0.0f;
GLfloat pitch = 0.0f;
GLfloat roll = 0.0f;

glm::vec3 RotationAxis = glm::vec3(0.0f, 0.0f, 0.0f);
GLfloat RotationAngle = 0.0f;

bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void Do_Movement();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For mac
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Animation Lab2 - Mengxia", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    glEnable(GL_DEPTH);
    
    //Connect shader
    
    Shader airplaneShader("plane.vert","plane.frag");
 //   Model airPlaneModel("blskesplaneobj/blskesplane.obj");

    Model airPlaneModel("AI/Aj.dae");
    
    while(!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime()*0.1;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        Do_Movement();

        // Rendering commands here
        glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glLoadIdentity();
      
        
        airplaneShader.Use();
        
        // view transformation
        glm:: mat4 view;
        view = glm:: lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        
        //projection
        glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
        
        
        
        glUniformMatrix4fv(glGetUniformLocation(airplaneShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(airplaneShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glm::mat4 model;
        // It's a bit too big for our scene, so scale it down
        // Using EulerAngle
        model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
        glm::mat4 rotationTransform = glm::eulerAngleYXZ(yaw, pitch, roll);
   //     model = rotationTransform * model;
     
        
        // Conversion frorm Euler angles (in radians) to Quaternion
        glm::quat MyQuaternion;
        glm::vec3 EulerAngles(pitch, yaw, roll);
        MyQuaternion = glm::quat(EulerAngles);
        glm::mat4 RotationMatrix = glm::toMat4(MyQuaternion);
        model = RotationMatrix * model;
        
        glUniformMatrix4fv(glGetUniformLocation(airplaneShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        airPlaneModel.Draw(airplaneShader);
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}



void Do_Movement()
{
    GLfloat cameraSpeed = 5.0f * deltaTime;
    if (keys[GLFW_KEY_W])
        cameraPos += cameraSpeed * cameraFront;
    if (keys[GLFW_KEY_S])
        cameraPos -= cameraSpeed * cameraFront;
    if (keys[GLFW_KEY_A])
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (keys[GLFW_KEY_D])
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    
    if (keys[GLFW_KEY_Y])
        yaw += 0.1;
    if (keys[GLFW_KEY_P])
        pitch += 0.1;
    if (keys[GLFW_KEY_R])
        roll += 0.1;
    if (keys[GLFW_KEY_L])
        pitch = 3.14159f / 2.0f; //gimbal-lock
    
    if (keys[GLFW_KEY_V]) {
        RotationAxis.x =1.0f;
        RotationAngle += 1.0f;
    }
    
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // When a user presses the escape key, we set the WindowShouldClose property to true,
    // closing the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    
    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
}




//bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;
    
    lastX = xpos;
    lastY = ypos;
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}
