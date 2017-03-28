//
//  LabAssignment3.cpp
//  OpenGL
//
//  Created by Mx on 12/02/2017.
//  Copyright © 2017 Mx. All rights reserved.
//


#include <string>
#include <algorithm>
#include <iostream>
#include <list>

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

using namespace std;


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

bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

float finger1Angle = 0.0f;
float finger2Angle = 0.0f;
float finger3Angle = 0.0f;

float finger1Anglel = 0.0f;
float finger2Anglel = 0.0f;
float finger3Anglel = 0.0f;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void Do_Movement();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);


// Bone class
class Bone {
    
private:
    string name;
    Bone* parent;
    vector<Bone*> children;
    float angle;
    glm::vec3 jointPosition;
    
    // delegation to Mesh
    Mesh* mesh;
    
public:
    // constructor
    Bone(string n, Mesh* m, Bone* p) {
        name = n;
        mesh = m;
        parent = p;
        angle = 0;
    }
    
    void setJointPosition(glm::vec3 p) {
        jointPosition = p;
    }

    void addChild(Bone b) {
        children.push_back(&b);
    }
    
    void setAngle(float a) {
        angle = a;
    }
    
    // method called get globaltransformation
    glm::mat4 getGlobalTransformation() {
        if (parent == NULL) {
            // root node
            return glm::mat4(1.0);
        } else {
            glm::mat4 parentGlobalTransformation = parent->getGlobalTransformation();
            // T(p2)*R2*T(-p2) * T(p1)*R1*T(-p1) *R*T
            glm::mat4 m = glm::translate(parentGlobalTransformation, jointPosition);
            m = glm::rotate(m, angle, glm::vec3(0.0, 0.0, 1.0));
            glm::vec3 minusJointPosition = glm::vec3(0.0)-jointPosition;
            m = glm::translate(m, minusJointPosition);
            
            return m;
        }
    }
    
    void renderObject(Shader shader) {
        glm::mat4 model = getGlobalTransformation();
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        mesh->Draw(shader);
    }
};


//Skeleton class
class Skeleton {

private:
    long boneNum;
    Bone* rootBone;
    vector<Bone*> bones;
    
public:
    Skeleton() {
        boneNum = 0;
    }
    
    void addRootBone(Bone* r) {
        rootBone = r;
        boneNum++;
        bones.push_back(r);
    }
    void addBone(Bone* b) {
        boneNum++;
        bones.push_back(b);
    }
    void render(Shader shader) {
        for (int i=0; i < boneNum; i++) {
            bones[i]->renderObject(shader);
        }
    }
};

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For mac
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Animation Lab3 - Mengxia", nullptr, nullptr);
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
    
    glEnable(GL_DEPTH);
    
    glm::mat4 model;
    
    //Connect shader
    Shader handShader("hand.vert","hand.frag");
    Model handModel("handModel/hand.dae");
    
    vector<Mesh> meshes = handModel.meshes;

    // hand base
    Bone base("base", &meshes[3], NULL);
    
    // add little finger
    Bone littleFinger1("little finger 1", &meshes[6], &base);
    littleFinger1.setJointPosition(glm:: vec3(0.045228, 0.07800698, -0.147849 ));
    base.addChild(littleFinger1);
    Bone littleFinger2("little finger 2", &meshes[13], &littleFinger1);
    littleFinger2.setJointPosition(glm::vec3( 0.119664, -0.109944, -0.147229 ));
    littleFinger1.addChild(littleFinger2);
    Bone littleFinger3("little finger 3", &meshes[8], &littleFinger2);
    littleFinger3.setJointPosition(glm::vec3(0.108498, -0.20361, -0.147229));
    littleFinger2.addChild(littleFinger3);
    
    // add ring Finger
    Bone ringFinger1("ring finger 1", &meshes[0], &base);
    ringFinger1.setJointPosition(glm:: vec3(0.147577, 0.07738596, -0.02875095));
    base.addChild(ringFinger1);
    Bone ringFinger2("ring finger 2", &meshes[9], &ringFinger1);
    ringFinger2.setJointPosition(glm::vec3(0.239382, -0.122351, -0.02875095));
    ringFinger1.addChild(ringFinger2);
    Bone ringFinger3("ring finger 3", &meshes[5], &ringFinger2);
     ringFinger3.setJointPosition(glm::vec3(0.283423, -0.271843, -0.02689099));
    ringFinger2.addChild(ringFinger3);
    
    // add middle Finger
    Bone middleFinger1("middle finger 1", &meshes[1], &base);
    middleFinger1.setJointPosition(glm:: vec3(0.1773509, 0.06870299, 0.110196));
    base.addChild(middleFinger1);
    Bone middleFinger2("middle finger 2", &meshes[12], &middleFinger1);
    middleFinger2.setJointPosition(glm::vec3(0.295209, -0.149643, 0.1039929));
    middleFinger1.addChild(middleFinger2);
    Bone middleFinger3("middle finger 3", &meshes[10], &middleFinger2);
    middleFinger3.setJointPosition(glm::vec3(0.359099, -0.32767, 0.102132));
    middleFinger2.addChild(middleFinger3);
    
    // add index Finger
    Bone indexFinger1("index finger 1", &meshes[4], &base);
    indexFinger1.setJointPosition(glm:: vec3(0.150679, 0.05877697, 0.237978));
    base.addChild(indexFinger1);
    Bone indexFinger2("index finger 2", &meshes[15], &indexFinger1);
      indexFinger2.setJointPosition(glm::vec3(0.264194, -0.113046, 0.239839));
    indexFinger1.addChild(indexFinger2);
    Bone indexFinger3("index finger 3", &meshes[7], &indexFinger2);
    indexFinger3.setJointPosition(glm::vec3(0.31816, -0.253854, 0.238598));
    indexFinger2.addChild(indexFinger3);

    // add thumb
    Bone thumb1("thumb 1", &meshes[2], &base);
    thumb1.setJointPosition(glm::vec3(-0.183043, 0.311861, 0.294426));
    base.addChild(thumb1);
    Bone thumb2("thumb 2", &meshes[11], &thumb1);
    thumb2.setJointPosition(glm::vec3(-0.179942, 0.12577, 0.36514));
    thumb1.addChild(thumb2);
    Bone thumb3("thumb 3", &meshes[14], &thumb2);
    thumb3.setJointPosition(glm::vec3(-0.119773, -0.01379799, 0.437715));
    thumb2.addChild(thumb3);
    
    
    
    Skeleton skeleton;
    skeleton.addRootBone(&base);
    skeleton.addBone(&littleFinger1);
    skeleton.addBone(&littleFinger2);
    skeleton.addBone(&littleFinger3);
    
    // add ringFinger on the base
    skeleton.addBone(&ringFinger1);
    skeleton.addBone(&ringFinger2);
    skeleton.addBone(&ringFinger3);
    
    // add middleFinger on the base
    skeleton.addBone(&middleFinger1);
    skeleton.addBone(&middleFinger2);
    skeleton.addBone(&middleFinger3);
    
    // add indexFinger on the base
    skeleton.addBone(&indexFinger1);
    skeleton.addBone(&indexFinger2);
    skeleton.addBone(&indexFinger3);
    
    // add thumb on the base
    skeleton.addBone(&thumb1);
    skeleton.addBone(&thumb2);
    skeleton.addBone(&thumb3);
    
    
    
    while(!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime()*0.1;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        glfwPollEvents();
        Do_Movement();
        
        
        // Rendering commands here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glLoadIdentity();
        
        
        handShader.Use();
        
        glm:: mat4 view;
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        
        //projection
        glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(handShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(handShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        
        // Conversion from Euler angles (in radians) to Quaternion
        glm::quat MyQuaternion;
        glm::vec3 EulerAngles(pitch, yaw, roll);
        MyQuaternion = glm::quat(EulerAngles);
        glm::mat4 RotationMatrix = glm::toMat4(MyQuaternion);
        model = RotationMatrix * model;
       
        
        // It's a bit too big for our scene, so scale it down
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
//        
        littleFinger1.setAngle(finger1Angle);
        littleFinger2.setAngle(finger2Angle);
        littleFinger3.setAngle(finger3Angle);
        
//finish
        ringFinger1.setAngle(finger1Angle);
        ringFinger2.setAngle(finger2Angle);
        ringFinger3.setAngle(finger3Angle);
        
       // first one finish
        middleFinger1.setAngle(finger1Angle);
        middleFinger2.setAngle(finger2Angle);
        middleFinger3.setAngle(finger3Angle);
//
        indexFinger1.setAngle(finger1Angle);
        indexFinger2.setAngle(finger2Angle);
        indexFinger3.setAngle(finger3Angle);
        
        thumb1.setAngle(finger1Angle);
        thumb2.setAngle(finger2Angle);
        thumb3.setAngle(finger3Angle);
        
        //draw hand model
        skeleton.render(handShader);
        
        
//        handModel.Draw(airplaneShader);
//        meshes[11].Draw(handShader);
//        meshes[14].Draw(handShader);
//        meshes[2].Draw(handShader);
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
    
    if (keys[GLFW_KEY_L]) {
        finger1Angle += 0.01;
        finger2Angle += 0.013;
        finger3Angle += 0.015;
     
    }
    
    if (keys[GLFW_KEY_K]) {
        finger1Angle -= 0.01;
        finger2Angle -= 0.013;
        finger3Angle -= 0.015;
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