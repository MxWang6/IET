//
//  LabAssignment44.cpp
//  OpenGL
//
//  Created by Mx on 08/03/2017.
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

float firstAngle = 0.0f;

glm::vec3 claviclePosition;
glm::vec3 upperArmPosition;
glm::vec3 forearmPosition;
glm::vec3 handPosition;
float splineAngle = 0.001;

glm::vec3 target(-0.2132, 1.0562, 0);
glm::vec3 endEffector;
glm::vec3 P1;
glm::vec3 P2;

// initial normal vector

glm::vec3 n_Vector;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void Do_Movement();
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
float calculateDistance(glm::vec3 Poin1, glm:: vec3 Point2);
float calculateAngle(float L1, float L2, float distance);
glm::vec3 normalVector(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
glm::vec3 CalculateAngle(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm:: vec3 T);
glm::vec3 DrawSpline(float t);

// Bone class
class Bone {
    
private:
    string name;
    Bone* parent;
    vector<Bone*> children;
    float angle = 0.0;
    glm::vec3 normal;
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
    
    void setNormal(glm::vec3 n) {
        normal = n;
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
            
            glm::mat4 m = parent->getGlobalTransformation();
            // T(p2)*R2*T(-p2) * T(p1)*R1*T(-p1) *R*T
            if (angle > 0.0) {
                m = glm::translate(m, jointPosition);
                m = glm::rotate(m, angle, normal);
                glm::vec3 minusJointPosition = glm::vec3(0.0)-jointPosition;
                m = glm::translate(m, minusJointPosition);
            }
            
            return m;
        }
    }
    
    void renderObject(Shader shader) {
        glm::mat4 model = getGlobalTransformation();
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        mesh->Draw(shader);
        
        for (int i = 0; i < children.size(); i++) {
            //children[i]->renderObject(shader);
            glm::mat4 m = children[i]->getGlobalTransformation();
            glm::vec3 childJoint = children[i]->getJoinPosition();
            childJoint = glm::vec3(m * glm::vec4(childJoint, 1.0f));
            children[i]->setJointPosition(childJoint);
        }
    }
};

vector<Bone*> chains;

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
    
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Animation Lab4 - Mengxia", nullptr, nullptr);
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
    
    //Bone body("body", &meshes[54], NULL);
    // hand base clavicle
    Bone base("base", &meshes[56], NULL);
    base.setJointPosition(glm:: vec3(-0.09079998, 1.4872, -0.02509999));
    //body.addChild(base);
    //chains.push_back(&base);
    
    //add upperarm
    Bone upperArm("upperArm", &meshes[12], &base);
    P1 = glm::vec3(-0.2075999, 1.454, -0.02359998);
    upperArm.setJointPosition(P1);
    base.addChild(upperArm);
    chains.push_back(&upperArm);
    
    Bone forearm("forearm", &meshes[32],&upperArm);
    P2 = glm::vec3(-0.4193, 1.2562, -0.02279996);
    forearm.setJointPosition(P2);
    upperArm.addChild(forearm);
    chains.push_back(&forearm);
    
    Bone hand("hand",&meshes[2],&forearm);
    hand.setJointPosition(glm::vec3(-0.587, 1.0565, 0.09579998));
    forearm.addChild(hand);
    //chains.push_back(&hand);
    
    
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
        
   
        upperArm.setJointPosition(glm::vec3(-0.2075999, 1.454, -0.02359998));
        forearm.setJointPosition(glm::vec3(-0.4193, 1.2562, -0.02279996));
        hand.setJointPosition(glm::vec3( -0.587, 1.0565, 0.09579998));
        
        //middleFinger1.setJointPosition(glm::vec3(-0.6508, 0.9827, 0.08950001));
        //middleFinger2.setJointPosition(glm::vec3(-0.6581, 0.9507, 0.1025));
        endEffector = glm::vec3( -0.587, 1.0565, 0.09579998); //glm::vec3(-0.6626, 0.9269, 0.1087);
        
        // CCD
        
//        while (glm::length(target - endEffector) > 0.01 ) {
            for (long i = chains.size()-1; i >=0; i--) {
                Bone* bone = chains[i];
                glm::vec3 P = bone->getJoinPosition();
                glm::vec3 t = target - P;
                glm::vec3 e = endEffector - P;
                float angle = acos(glm::dot(t, e)/(glm::length(t) * glm::length(e)));
                glm::vec3 n = normalVector(P, target, endEffector);
                bone->setAngle(angle);
                bone->setNormal(-n);
                bone->renderObject(handShader);
                
                glm::mat4 m = bone->getGlobalTransformation();
                endEffector = glm::vec3(m * glm::vec4(endEffector, 1.0f));
            }
//        }
        
        
        
        glUniformMatrix4fv(glGetUniformLocation(handShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        meshes[56].Draw(handShader);
        // draw the rest of the man model
        for (int i=0;i< meshes.size(); i++) {
            if( i == 56 || i == 12 || i ==32 ||i ==2 ||i == 16 || i == 22 || i ==26 ||i ==29||i == 40 || i == 50 || i ==58 ||i ==68||i == 70 || i == 74 || i ==10 ||i ==60 || i ==53 || i==77 || i==14 ){
                
            }
            else{
                meshes[i].Draw(handShader);
            }
        }
        
        
        
        
        // draw the position of the ball - spline
//        glm:: mat4 model1;
//        model1 = glm::translate(model1, glm::vec3(target));
//        glm::vec3 center(0, 1.0562, 0);
//        model1 = glm::translate(model1, center);
//        model1 = glm::rotate(model1, splineAngle, glm::vec3(0.0, 0.0, 1.0));
//        glm::vec3 minusCenter = glm::vec3(0.0)-center;
//        model1 = glm::translate(model1, minusCenter);
//        model1 = glm::scale(model1, glm::vec3(0.001f, 0.001f, 0.001f));
//         target = glm::vec3(model1 * glm::vec4(target, 1.0f));
//        glUniformMatrix4fv(glGetUniformLocation(ballShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model1));
//        ballModel.Draw(ballShader);
        
        
        // draw the position of the ball
        glm:: mat4 model1;
        model1 = glm::translate(model1, glm::vec3(target));
        model1 = glm::scale(model1, glm::vec3(0.001f, 0.001f, 0.001f));
        glUniformMatrix4fv(glGetUniformLocation(ballShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model1));
        ballModel.Draw(ballShader);

       
        
        
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}


glm::vec3 DrawSpline(float t){
    
    glm::vec3 Position;
    glm::vec3 P1(-1.0f,1.8f,1.0f);
    glm::vec3 P2(-0.5f,0.8f,1.0f);
    glm::vec3 P3(0.7f,0.348f,1.0f);
    glm::vec3 P4(1.0f,1.8f,2.0f);
    
    
  // Position = pow((1-t), 3) * p
    P1.x = pow((1-t),3) * P1.x + 3*t * pow((1-t), 2) * P2.x + 3*t *t * pow((1-t), 2) * P3.x + pow(t,3) * P4.x;
    
    P1.y = pow((1-t),3) * P1.y + 3*t * pow((1-t), 2) * P2.y + 3*t *t * pow((1-t), 2) * P3.y + pow(t,3) * P4.y;
    
    P1.z = pow((1-t),3) * P1.z + 3*t * pow((1-t), 2) * P2.z + 3*t *t * pow((1-t), 2) * P3.z + pow(t,3) * P4.z;
    
    return Position;
    
}



glm::vec3 CalculateAngle(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm:: vec3 T){
    
    float L1 = calculateDistance(p0, p1);
    float L2 = calculateDistance(p1, p2);
    
    float m;
    float r; // radius
    glm:: vec3 R;
    glm:: vec3 K; // the first rotate angle
    
    float d = calculateDistance(p0, T);
    m= (pow(d, 2)-pow(L2, 2)+pow(L1, 2))/(2*d);
    R = (m * (T-p0))/d + p0;
    r = abs(sqrt(pow(L1, 2)-pow(m, 2)));
    
   // K = (T * R)/(T-p0);
    
    glm::vec3 T_decrease_P0;
    glm::vec3 K_decrease_R;
    
    T_decrease_P0 = T-p0;
    
    float Q = T_decrease_P0.x *(K.x - R.x) + T_decrease_P0.y * (K.y - R.y);
    
     float x=0;
     float y=0;
    
     for (x=-1.0f; x<1;){
        for(y=-1.0f; y<1;){
            
            int a = pow((R.x -x),2) + pow((R.y - y), 2) + pow(Q, 2) / pow(T_decrease_P0.z, 2);
            if ( a == 0) {
                
                printf ("x is %f. y is %f.\n", x,y);
                 printf ("qqqqqqq");
                break;
            }
            y += 0.001;
        }
          x += 0.001;
    }
    
    K.x = x;
    K.y = y;
    K.z = R.z - (T_decrease_P0.x *(K.x - R.x) + T_decrease_P0.y * (K.y - R.y))/T_decrease_P0.z;

    
    float P1_K = calculateDistance(p0, K);
    float K_P1 = calculateDistance(K, p1);
    
    float angle_arc = calculateAngle(L1, K_P1, P1_K);
    
    firstAngle = acos(angle_arc) * 180 / PI;
    
    glm :: vec3 NV = normalVector(p0, p1, K);
    
    return NV;
    
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
    
    //T F G H: target controller
    if (keys[GLFW_KEY_T])
        target.y += 0.01;
    if (keys[GLFW_KEY_F])
        target.x -= 0.01;
    if (keys[GLFW_KEY_G])
        target.y -= 0.01;
    if (keys[GLFW_KEY_H])
        target.x += 0.01;
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

