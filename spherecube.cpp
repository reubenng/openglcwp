#include "spherecube.h"

vector<GLfloat>* sphere(vector<GLfloat>* verts, vector<GLint>* ind, vector<GLfloat>* normals, GLint rings, GLint segments, GLint r){ // UV sphere
	for(int i = 0; i <= rings; i++){ // for every ring around the sphere
		// calculating theta
		float v = i / (float) rings;
		float t = (v * M_PI) - (M_PI / 2); // -90 < theta < 90
		
		for(int j = 0; j <= segments; j++){ // each segment in a ring
			// calculating phi
			float u = j / (float) segments;
			float p = u * 2 * M_PI; // 0 < phi < 360

			// refered to sphere generation http://paulbourke.net/geometry/circlesphere/
			// equations of points on sphere surface
			float x = r * cosf(t) * cosf(p); // x = cos theta cos phi
			float y = r * sinf(t); // y = sin theta
			float z = r * cosf(t) * sinf(p); // z = cos theta sin phi

			// verts structure [vertx, vertx, vertx, normx, normx, normx, texcoordx, texcoordy]
			glm::vec3 vertex(x, y, z); // sphere vertex
			// normalise points
			// add new points to the end of vector, 1 vert every 3 points
			verts->push_back(vertex.x);
			verts->push_back(vertex.y);
			verts->push_back(vertex.z);

			// create normal vertex just for drawing purpose, not used for shading
			// points joining to vertices of sphere to create normal lines
			normals->push_back(vertex.x); 
			normals->push_back(vertex.y); 
			normals->push_back(vertex.z);

			// calculating normals
			x = x + x * 0.1; // normal is parallel to distance from centre
			y = y + y * 0.1;
			z = z + z * 0.1;
			glm::vec3 normal(x, y, z); 
			verts->push_back(normal.x);
			verts->push_back(normal.y);
			verts->push_back(normal.z);

			normals->push_back(normal.x);
			normals->push_back(normal.y);
			normals->push_back(normal.z);

            verts->push_back (v); // texcord for texture
            verts->push_back (u);
		}
	}
	// indices for drawing sequences
	for (int i = 0; i < segments * rings + segments; i++){
        ind->push_back (i);  // first point on triangle
        ind->push_back (i + segments + 1); // point below next point
        ind->push_back (i + 1); // next point 

        ind->push_back (i); // the other triangle to form a rectangle
        ind->push_back (i + segments);
        ind->push_back (i + segments + 1);
    }
}

vector<GLfloat>* cube(vector<GLfloat>* verts, vector<GLint>* ind, GLint cubesize){
	GLint half = cubesize / 2;

	verts->push_back(-half);
	verts->push_back(0);
	verts->push_back(-half);
	verts->push_back(-half);
	verts->push_back(cubesize);
	verts->push_back(-half);
	verts->push_back(half);
	verts->push_back(cubesize);
	verts->push_back(-half);
	verts->push_back(half);
	verts->push_back(0);
	verts->push_back(-half);

	verts->push_back(-half);
	verts->push_back(0);
	verts->push_back(half);
	verts->push_back(-half);
	verts->push_back(cubesize);
	verts->push_back(half);
	verts->push_back(half);
	verts->push_back(cubesize);
	verts->push_back(half);
	verts->push_back(half);
	verts->push_back(0);
	verts->push_back(half);

    ind->push_back (0); 
    ind->push_back (1); 
    ind->push_back (2); 
    ind->push_back (3);
    ind->push_back (0);   

    ind->push_back (3); 
    ind->push_back (7); 
    ind->push_back (6); 
    ind->push_back (2);  
    
    ind->push_back (6); 
    ind->push_back (5); 
    ind->push_back (4); 
    ind->push_back (7);  
    
    ind->push_back (4); 
    ind->push_back (0); 
    ind->push_back (1); 
    ind->push_back (5);
    ind->push_back (4);   
}