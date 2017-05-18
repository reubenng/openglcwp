#include "utils.h"

char* filetobuf(char *file){
	FILE *fptr;
	long length;
    char *buf;
    fptr = fopen(file, "r"); // open file
    if(!fptr) cout << "Fail to open " << file << endl;
    fseek(fptr, 0, SEEK_END); // Seek to the end of the file
    length = ftell(fptr); /* Find out how many bytes into the file we are */
    buf = (char*)malloc(length + 1); /* Allocate a buffer for the entire length of the file and a null terminator */
    fseek(fptr, 0, SEEK_SET); /* Go back to the beginning of the file */
    fread(buf, length, 1, fptr); /* Read the contents of the file in to the buffer */
    fclose(fptr); /* Close the file */
    buf[length] = 0; /* Null terminator */
    return buf; /* Return the buffer */
}

void check(char *where){
    const char *what;
    int err = glGetError();
    if(!err) return;
    if(err == GL_INVALID_ENUM) what = "GL_INVALID_ENUM";
    else if(err == GL_INVALID_VALUE) what = "GL_INVALID_VALUE";
    else if(err == GL_INVALID_OPERATION) what = "GL_INVALID_OPERATION";
    else if(err == GL_INVALID_FRAMEBUFFER_OPERATION) what = "GL_INVALID_FRAMEBUFFER_OPERATION";
    else if(err == GL_OUT_OF_MEMORY) what = "GL_OUT_OF_MEMORY";
    else what = "Error Unknown";
    cout << "Error (" << err << ") " << what << " at " << where << endl;
    exit(1);
}