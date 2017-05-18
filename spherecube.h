#ifndef M_PI
#define M_PI 3.14159
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <vector>
#include <iostream>
using namespace std;

vector<GLfloat>* sphere(vector<GLfloat>* verts, vector<GLint>* ind, vector<GLfloat>* normals, GLint rings, GLint segments, GLint r);

vector<GLfloat>* cube(vector<GLfloat>* verts, vector<GLint>* ind, GLint cubesize);