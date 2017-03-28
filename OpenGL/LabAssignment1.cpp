//
//  main.cpp
//  OpenGL
//
//  Created by Mx on 21/01/2017.
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

// GLM Mathemtics
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For mac
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Animation Lab1", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    //Connect shader
    
    Shader triangleShader("triangle.vs","triangle.frag");
    
    GLfloat vertices[] = {
        // Positions         // Colors
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // Bottom Right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // Bottom Left
        0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // Top
    };
    
    GLuint VBO;// Vertex Buffer object
    glGenBuffers(1, &VBO);
    
    GLuint VAO; // Vertex Array Object
    glGenVertexArrays(1, &VAO);
    
    glBindVertexArray(VAO); // Array type
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // buffer type
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // pass triangle data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3* sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    
    
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        
        // Rendering commands here
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
     
        triangleShader.Use();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
       
        glBindVertexArray(0);
        
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // When a user presses the escape key, we set the WindowShouldClose property to true,
    // closing the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}