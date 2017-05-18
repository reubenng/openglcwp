//============================================================================
// Name        : cwp.cpp
// Author      : rdcn1g14
// Description : COMP3214 Coursework P
/*
A program that bounces at least 2 spheres and one cube around a cubic 
arena with gravity off and a coefficient of restitution of 1.0, 
i.e. perfect loss less bounces.
*/
//============================================================================

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "World.h"
#include "spherecube.h"
#include "utils.h"

const double PI = 3.141592653589793;
const double PIo2 = PI/2.;
const double PIo4 = PI/4.;
const double PI2 = PI * 2.;
const float lod = PI/32.;

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
/*
 * Set up bullet - globals.
 */
#include <btBulletDynamicsCommon.h>

GLuint spherevbo, spherevao, sphereebo, normalvao, normalvbo;
GLuint fragmentshader, vertexshader, shaderprogramme;
GLuint conevbo, conevao, cubeebo; // for cones
GLuint boxvbo, boxvao, boxebo; // for cones
vector<GLfloat> spherevert, norm, conevert, boxvert; // for storing sphere vertices and normal
vector<GLint> ind, indc, indb; // drawing indices
GLint rings = 7; // number of rings and segments that make sphere
GLint segments = 14;
GLint cubesize = 5;
GLint r = 5;

btBroadphaseInterface* broadphase;
btDefaultCollisionConfiguration* collisionConfiguration;
btCollisionDispatcher* dispatcher;
btSequentialImpulseConstraintSolver* solver;
btDiscreteDynamicsWorld* dynamicsWorld;

std::vector<btRigidBody*> MovingBits; // so that can get at all bits
std::vector<btRigidBody*> StaticBits; // especially during clean up.
/*
 * Bullet Code
 */
btRigidBody* SetSphere(float size, btTransform T) {
    btCollisionShape* fallshape = new btSphereShape(size);
    btDefaultMotionState* fallMotionState = new btDefaultMotionState(T);
    btScalar mass = 1;
    btVector3 fallInertia(0,0,0);
    fallshape ->calculateLocalInertia(mass,fallInertia);
    btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,fallMotionState,fallshape,fallInertia);
    btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);
    fallRigidBody->setLinearVelocity(btVector3(-20, 30, 0));
    fallRigidBody->setRestitution(COE);
    dynamicsWorld->addRigidBody(fallRigidBody);
    return fallRigidBody;
    }
void bullet_init() {
    /*
     * set up world
     */
    broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver;
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0., GRAVITY, 0));

    btVector3 normals[] = {
        btVector3(0.f, 1.f, 0.f),
        btVector3(1.f, 0.f, 0.f),
        btVector3(-1.f, 0.f, 0.f),
        btVector3(0.f, -1.f, 0.f),
        btVector3(0.f, 0.f, -1.f),
        btVector3(0.f, 0.f, 1.f),
    };

    btVector3 pos[] = {
        btVector3(0.f, -1.f, 0.f),
        btVector3(-20.f, 0.f, 0.f),
        btVector3(20.f, 0.f, 0.f),
        btVector3(0.f, 40.f, 0.f),
        btVector3(0.f, 0.f, 20.f),
        btVector3(0.f, 0.f, -20.f),
    };
    
    for (unsigned int i = 0; i < 6; i++) { // set up walls
        btCollisionShape* shape = new btStaticPlaneShape(normals[i], 0.f);
        btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), pos[i]));
        btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(0,motionState,shape,btVector3(0,0,0));
        btRigidBody* rigidBody = new btRigidBody(rigidBodyCI);
        rigidBody->setRestitution(COE);
        dynamicsWorld->addRigidBody(rigidBody);
    }
     // Set up sphere 0
    MovingBits.push_back(SetSphere(5., btTransform(btQuaternion(0,0,0,1),btVector3(0,20,10))));
     // Set up sphere 1
    MovingBits.push_back(SetSphere(5., btTransform(btQuaternion(0,0,0,2),btVector3(-10,50,-30))));
     // Set up sphere 2
    MovingBits.push_back(SetSphere(5., btTransform(btQuaternion(0,0,0,2),btVector3(-10,40,0))));
    }

glm::vec3 bullet_step(int i) {
    btTransform trans;
    btRigidBody* moveRigidBody;
    int n = MovingBits.size();
    moveRigidBody = MovingBits[i];
    dynamicsWorld->stepSimulation(1/60.f,10);
    moveRigidBody->getMotionState()->getWorldTransform(trans);
    return glm::vec3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ());
    }

void bullet_close() {
    /*
     * This is very minimal and relies on OS to tidy up.
     */
    btRigidBody* moveRigidBody;
    moveRigidBody = MovingBits[0];
    dynamicsWorld->removeRigidBody(moveRigidBody);
    delete moveRigidBody->getMotionState();
    delete moveRigidBody;
    delete dynamicsWorld;
    delete solver;
    delete collisionConfiguration;
    delete dispatcher;
    delete broadphase;
    }

void setupshaders(){
    int length;
    char text[512];
    GLint success;
    // vertex display
    const char* vertex_shader = filetobuf((char*)"./main.vert");
    vertexshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexshader, 1, &vertex_shader, NULL);
    glCompileShader(vertexshader);
    glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &success);
    glGetShaderInfoLog(vertexshader, 512, NULL, text);
    if(!success) cout << "Validate vertexshader " << text << endl;

    // colour surface
    const char* fragment_shader = filetobuf((char*)"./main.frag");
    fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentshader, 1, &fragment_shader, NULL);
    glCompileShader(fragmentshader);
    glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &success);
    glGetShaderInfoLog(fragmentshader, 512, NULL, text);
    if(!success) cout << "Validate fragmentshader " << text << endl;

    shaderprogramme = glCreateProgram(); // create empty program
    glAttachShader(shaderprogramme, fragmentshader); // attach both
    glAttachShader(shaderprogramme, vertexshader);
    glLinkProgram(shaderprogramme); // link together

    glGetProgramInfoLog(shaderprogramme, 512, &length, text);
    if(length > 0) cout << "Validate shaderprogramme " << text << endl;
    glUseProgram(shaderprogramme); // use program
    glDeleteShader(vertexshader); // delete linked shader objects
    glDeleteShader(fragmentshader); 

    // check((char*)"setupshaders");
    }
void setupbox(){
// box
    cube(&boxvert, &indb, 40);

    glGenVertexArrays(1, &boxvao); // generate vao for cones
    glGenBuffers(1, &boxvbo); // generate vbo
    glGenBuffers(1, &boxebo); // index for drawing cube
    glBindVertexArray(boxvao); // set to current vao
    glBindBuffer(GL_ARRAY_BUFFER, boxvbo); // set to current vbo
    glBufferData(GL_ARRAY_BUFFER, boxvert.size() * sizeof(GLfloat), &boxvert[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxebo); // for drawing sequence
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indb.size() * sizeof(GLint), &indb[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL);
    glEnableVertexAttribArray(0); // attribute 0
}

void setupgeometry(){
    // sphere
    sphere(&spherevert, &ind, &norm, rings, segments, r);
    
    glGenVertexArrays(1, &spherevao); // generate vao
    glGenBuffers(1, &spherevbo); // generate vbo
    glGenBuffers(1, &sphereebo); // index for drawing sphere

    glGenVertexArrays(1, &normalvao); // generate vao
    glGenBuffers(1, &normalvbo); // generate vbo for normal vertices

    glBindVertexArray(spherevao); // set to current vao
    glBindBuffer(GL_ARRAY_BUFFER, spherevbo); // set to current vbo
    glBufferData(GL_ARRAY_BUFFER, spherevert.size() * sizeof(GLfloat), &spherevert[0], GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereebo); // for drawing sequence
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof(GLint), &ind[0], GL_STATIC_DRAW);
    
    // attribute 0, [vertex, vertex, vertex, normal, normal, normal, texcoord, texcoord]
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), NULL);
    glEnableVertexAttribArray(0); // attribute 0
    // attribute 1 for normal, this normal is for calculating shading
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1); // attribute 1
    // attribute 2 for texture, every 2 points
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2); // attribute 2

    // cube
    cube(&conevert, &indc, cubesize);

    glGenVertexArrays(1, &conevao); // generate vao for cones
    glGenBuffers(1, &conevbo); // generate vbo
    glGenBuffers(1, &cubeebo); // index for drawing cube
    glBindVertexArray(conevao); // set to current vao
    glBindBuffer(GL_ARRAY_BUFFER, conevbo); // set to current vbo
    glBufferData(GL_ARRAY_BUFFER, conevert.size() * sizeof(GLfloat), &conevert[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeebo); // for drawing sequence
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indc.size() * sizeof(GLint), &indc[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL);
    glEnableVertexAttribArray(0); // attribute 0

    glBindVertexArray(0); // unbind
    // check((char*)"setupgeometry");
    }

void render(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear screen

    glm::mat4 Projection = glm::mat4(1.0f);
    Projection = glm::ortho(-35., 35., -25., 50., -50., 50.);
    glm::mat4 View = glm::mat4(1.);
    View = glm::lookAt(glm::vec3(10, 15, 20), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 2.0f, 0.0f));
    glm::mat4 Model = glm::mat4(1.);

    glUniformMatrix4fv(glGetUniformLocation(shaderprogramme, "Projection"), 1, GL_FALSE, glm::value_ptr(Projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderprogramme, "View"), 1, GL_FALSE, glm::value_ptr(View));
    glUniformMatrix4fv(glGetUniformLocation(shaderprogramme, "Model"), 1, GL_FALSE, glm::value_ptr(Model));
    
    glBindVertexArray(boxvao); // set vao as input for drawing
    glDrawElements(GL_LINE_STRIP, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0); // unbind

    for(int i = 0 ; i < 2; i++) { // loop over shapes
        glm::vec3 position = bullet_step(i);
        Model = glm::translate(position);
        glUniformMatrix4fv(glGetUniformLocation(shaderprogramme, "Model"), 1, GL_FALSE, glm::value_ptr(Model));
        
        // wireframe sphere
        glBindVertexArray(spherevao); // set vao as input for drawing
        glDrawElements(GL_LINE_STRIP, ind.size(), GL_UNSIGNED_INT, 0);
    }
        glm::vec3 position = bullet_step(2);
        Model = glm::translate(position);
        glUniformMatrix4fv(glGetUniformLocation(shaderprogramme, "Model"), 1, GL_FALSE, glm::value_ptr(Model));
        glBindVertexArray(conevao); // set vao as input for drawing
        glDrawElements(GL_LINE_STRIP, 36, GL_UNSIGNED_INT, 0);
    
    glFlush();
    glBindVertexArray(0); // unbind
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

int main( void ) {
    int k = 0;
    bool good;
    GLFWwindow* window;
    if( !glfwInit() ) {
        printf("Failed to start GLFW\n");
        exit( EXIT_FAILURE );
        }
    window = glfwCreateWindow(640, 640, "Visible axies", NULL, NULL);
    if (!window) {
        glfwTerminate();
        printf("GLFW Failed to start\n");
        return -1;
        }
    /* Make the window's context current */
    glfwMakeContextCurrent(window); // IMPORTANT: Must be done so glew recognises OpenGL
    glewExperimental = GL_TRUE;
    int err = glewInit();
    if (GLEW_OK != err) {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        }
    fprintf(stderr, "Glew done\n");
    glfwSetKeyCallback(window, key_callback);
    fprintf(stderr, "GL INFO %s\n", glGetString(GL_VERSION));

    bullet_init(); // set up physics

    glEnable(GL_DEPTH_TEST);
    setupshaders();
    setupbox();
    setupgeometry();

    glClearColor(0.0, 0., 0., 1.0);/* Make our background black */
    while(!glfwWindowShouldClose(window)) { // Main loop
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render();
        glFlush();
        glfwSwapBuffers(window);// Swap front and back rendering buffers
        glfwPollEvents(); // Poll for events.
        }
    void bullet_close();
    glfwTerminate();// Close window and terminate GLFW
    exit( EXIT_SUCCESS );// Exit program
    }