//
//  LabAssignment4.cpp
//  OpenGL
//
//  Created by Mx on 23/02/2017.
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


#define PI 3.14159265

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

float theT1 = 0.0f;
float theT2 = 0.0f;
float thetaT =0.0f;
float f_Test = 0.0f;
float q= 0.0f;

glm::vec3 claviclePosition;
glm::vec3 upperArmPosition;
glm::vec3 forearmPosition;
glm::vec3 handPosition;

glm::vec3 endEffector(-0.2132, 1.0562, 0);

// initial normal vector

glm::vec3 n_Vector;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void Do_Movement();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
float calculateDistance(glm::vec3 Poin1, glm:: vec3 Point2);
float calculateAngle(float L1, float L2, float distance);
glm::vec3 normalVector(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);

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
    
    glm::vec3 getJoinPosition(){
        
        return jointPosition;
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
    //    model = glm::translate(model, glm::vec3(0.0f,-1.0f,0.0f));
    //    model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
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
    Shader ballShader("hand.vert","hand.frag");
    Model handModel("man/manmodel.dae");
    Model ballModel("football/football-soccer-ball.obj");
  
    vector<Mesh> meshes = handModel.meshes;
    
    // hand base clavicle
    Bone base("base", &meshes[56], NULL);
    base.setJointPosition(glm:: vec3(-0.09079998, 1.4872, -0.02509999));
    //add upperarm
    
    Bone upperArm("upperArm", &meshes[12], &base);
    upperArm.setJointPosition(glm::vec3(-0.2075999, 1.454, -0.02359998));
    base.addChild(upperArm);
    Bone forearm("forearm", &meshes[32],&upperArm);
    forearm.setJointPosition(glm::vec3(-0.4193, 1.2562, -0.02279996));
    upperArm.addChild(forearm);
    Bone hand("hand",&meshes[2],&forearm);
    hand.setJointPosition(glm::vec3( -0.587, 1.0565, 0.09579998));
    forearm.addChild(hand);
    
    // add little finger
    Bone littleFinger1("little finger 1", &meshes[16], &hand);
    hand.addChild(littleFinger1);
    Bone littleFinger2("little finger 2", &meshes[29], &littleFinger1);
    littleFinger1.addChild(littleFinger2);
    Bone littleFinger3("little finger 3", &meshes[26], &littleFinger2);
    littleFinger2.addChild(littleFinger3);
    
    // add ring Finger
    Bone ringFinger1("ring finger 1", &meshes[22], &hand);
    hand.addChild(ringFinger1);
    Bone ringFinger2("ring finger 2", &meshes[70], &ringFinger1);
    ringFinger1.addChild(ringFinger2);
    Bone ringFinger3("ring finger 3", &meshes[68], &ringFinger2);
    ringFinger2.addChild(ringFinger3);

    // add middle Finger
    Bone middleFinger1("middle finger 1", &meshes[50], &hand);
    hand.addChild(middleFinger1);
    Bone middleFinger2("middle finger 2", &meshes[58], &middleFinger1);
    middleFinger1.addChild(middleFinger2);
    Bone middleFinger3("middle finger 3", &meshes[14], &middleFinger2);
    middleFinger2.addChild(middleFinger3);
    
    // add index Finger
    Bone indexFinger1("index finger 1", &meshes[53], &hand);
    hand.addChild(indexFinger1);
    Bone indexFinger2("index finger 2", &meshes[10], &indexFinger1);
    indexFinger1.addChild(indexFinger2);
    Bone indexFinger3("index finger 3", &meshes[74], &indexFinger2);
    indexFinger2.addChild(indexFinger3);

    // add thumb
    Bone thumb1("thumb 1", &meshes[40], &hand);
    hand.addChild(thumb1);
    Bone thumb2("thumb 2", &meshes[77], &thumb1);
    thumb1.addChild(thumb2);
    Bone thumb3("thumb 3", &meshes[60], &thumb2);
    thumb2.addChild(thumb3);

    // add all hand to skeleton
    Skeleton skeleton;
    skeleton.addRootBone(&base);
    skeleton.addBone(&upperArm);
    skeleton.addBone(&forearm);
    skeleton.addBone(&hand);
    
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
        glm:: mat4 model;
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        
        //projection
        glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(handShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(handShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        
        // ball
        glUniformMatrix4fv(glGetUniformLocation(ballShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(ballShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        
        
        // Conversion from Euler angles (in radians) to Quaternion
        glm::quat MyQuaternion;
        glm::vec3 EulerAngles(pitch, yaw, roll);
        MyQuaternion = glm::quat(EulerAngles);
        glm::mat4 RotationMatrix = glm::toMat4(MyQuaternion);
        model = RotationMatrix * model;
        
        // It's a bit too big for our scene, so scale it down
//        model = glm::translate(model, glm::vec3(0.0f,-1.0f,0.0f));
//        model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
     // model = glm::translate(model, glm::vec3(1.0f,-3.0f,0.0f));
        
     //   base.setAngle(finger3Angle);
         upperArm.setAngle(finger1Angle);
        forearm.setAngle(finger2Angle);
         hand.setAngle(finger3Angle);

        // calculate the angle
       
        glm::vec3 claviclePosition = base.getJoinPosition();
        glm::vec3 upperArmPosition = upperArm.getJoinPosition();
        glm::vec3 forearmPosition = forearm.getJoinPosition();
        glm::vec3 handPosition = hand.getJoinPosition();
       
        glm::vec3 N = normalVector(upperArmPosition, forearmPosition, handPosition);
        
        
        float distance = calculateDistance(upperArmPosition, endEffector);
        float x = abs(endEffector.x - upperArmPosition.x);
        float thetaT = acos(x/distance);
        printf ("The arc cosine of %f is %f degrees.\n", distance,thetaT);
        
        
        float L1 = calculateDistance(upperArmPosition, forearmPosition);
        float L2 = calculateDistance(forearmPosition, handPosition);
        
        printf("upper arm + %f \n",L1);
        printf("lower arm + %f \n",L2);
        
      //  printf ("L1 is %f degrees and L2 is %f.\n", L1,L2);
        float a = (L1*L1+distance*distance-L2*L2)/(2*L1*distance);
        theT1 = (acos(a)+thetaT); //* 180 /PI;
        
        float b = (L1*L1+L2*L2-(distance*distance))/(2*L1*L2);
        theT2 = acos(b);//* 180/PI;
        
        
        //calculate the original hand position wrong
        float L_clavicleToUpperarm = calculateDistance(upperArmPosition, claviclePosition);
        float L_forearmToUpperarm = calculateDistance(upperArmPosition, forearmPosition);
        float L_clavicleToForearm = calculateDistance(forearmPosition, claviclePosition);
        
        float Test1 = acos(calculateAngle(L_clavicleToUpperarm, L_forearmToUpperarm, L_clavicleToForearm));
        
        f_Test = Test1; //*180/PI;
        
        
        // float D_upperarmTofoream = calculateDistance(upperArmPosition, forearmPosition);
        float D_angle = atan(abs(forearmPosition.x)/abs(forearmPosition.y)) * 180/PI;
        
        q = (180-theT1*180/PI-D_angle)*PI/180;
        
        meshes[7].Draw(handShader);
        
        
        //draw hand model
       skeleton.render(handShader);
       glUniformMatrix4fv(glGetUniformLocation(handShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
       //    handModel.Draw(handShader);
        
        // draw the rest of the man model
        for (int i=0;i< meshes.size(); i++) {
            if( i == 56 || i == 12 || i ==32 ||i ==2 ||i == 16 || i == 22 || i ==26 ||i ==29||i == 40 || i == 50 || i ==58 ||i ==68||i == 70 || i == 74 || i ==10 ||i ==60 || i ==53 || i==77 || i==14 ){
               
            }
            else{
                 meshes[i].Draw(handShader);
            }
        }
        
        
        glm:: mat4 model1;
        // model1 = glm::translate(model1, glm::vec3(-0.6579868, 0.9, 0.6827797));
        model1 = glm::translate(model1, glm::vec3(endEffector));
        model1 = glm::scale(model1, glm::vec3(0.001f, 0.001f, 0.001f));
        glUniformMatrix4fv(glGetUniformLocation(ballShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model1));
        ballModel.Draw(ballShader);

        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}


float calculateDistance(glm::vec3 p1, glm:: vec3 p2)
{
    float distance=0;
    
    distance=abs(sqrt(pow((p2.x-p1.x),2)+pow((p2.y-p1.y),2)+pow((p2.z-p1.z),2)));
    return distance;
}

float calculateAngle(float L1, float L2, float distance)
{
    float arc = 0;
    
    arc = (pow(L1, 2)+pow(L2, 2)-pow(distance, 2))/(2*L1*L2);
    
    return arc;
}

//Nx = UyVz - UzVy

//Ny = UzVx - UxVz
//
//Nz = UxVy - UyVx

glm::vec3 normalVector(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3){
    
    glm::vec3 normal;
    
    glm::vec3 U = p2 - p1;
    glm::vec3 V = p3 - p1;
    
    normal.x = U.y*V.z - U.z*V.y;
    normal.y = U.z*V.x - U.x*V.z;
    normal.z = U.x*V.y - U.y*V.x;
    
    
    return normal;
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
        finger1Angle = q;//(f_Test - theT1);
        finger2Angle = theT2;
     //   finger3Angle += 0.015;
        
    }
    
    if (keys[GLFW_KEY_K]) {
        finger1Angle -= theT1;
        finger2Angle -= theT2;
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

