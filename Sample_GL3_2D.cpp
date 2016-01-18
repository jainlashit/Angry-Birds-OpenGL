#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define SCREEN_HEIGHT 480
#define SCREEN_WIDTH 1280  
#define GROUND_HEIGHT 50
#define BIRD_MARGIN 30
#define CANON_WHEEL_CENTERX 120
#define CANON_WHEEL_CENTERY 70
#define CANON_TUNNEL_LENGTH 70
#define OBSTACLE_STARTSX 800.0f
#define OBSTACLE_ICE_SIZE 50.0f

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
float canon_tunnel_rotation = 0;
float canon_tunnel_angle = 0;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_X:
                // do something ..
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                canon_tunnel_rotation = 0.0f;
            else if(action == GLFW_PRESS)
              canon_tunnel_rotation = 0.01f;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE)
                canon_tunnel_rotation = 0.0f;
            else
              canon_tunnel_rotation = -0.01f;
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(0.0f, (float)(SCREEN_WIDTH), 0.0f, (float)SCREEN_HEIGHT, 0.0f, 500.0f);
}

VAO* drawCircle(GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides, GLfloat red, GLfloat blue, GLfloat green)
{
  GLint numberOfVertices = numberOfSides + 2;

  GLfloat doublePI = 2.0f * M_PI;
  
  GLfloat circleVerticesX[numberOfVertices];
  GLfloat circleVerticesY[numberOfVertices];
  GLfloat circleVerticesZ[numberOfVertices];

  circleVerticesX[0] = x;
  circleVerticesY[0] = y;
  circleVerticesZ[0] = z;

  for (int i = 1; i < numberOfVertices; i++)
  {
    circleVerticesX[i] = x + (radius * cos(i * doublePI/numberOfSides));
    circleVerticesY[i] = y + (radius * sin(i * doublePI/numberOfSides));
    circleVerticesZ[i] = z;
  }

  GLfloat allVertices[numberOfVertices * 3];
  GLfloat allColors[numberOfVertices * 3];
  for (int i = 0; i < numberOfVertices; i++)
  {
    allVertices[i * 3] = circleVerticesX[i];
    allColors[i * 3] = red;
    allVertices[(i * 3) + 1] = circleVerticesY[i];
    allColors[(i * 3) + 1] = blue;
    allVertices[(i * 3) + 2] = circleVerticesZ[i];
    allColors[(i * 3) + 2] = green;
  }

  return create3DObject(GL_TRIANGLE_FAN, numberOfVertices, allVertices, allColors, GL_FILL);
}

VAO* drawRectangle(GLfloat x, GLfloat y, GLfloat z, GLfloat halfLength, GLfloat halfWidth, GLfloat red, GLfloat blue, GLfloat green, bool fill_mode) 
{
  cout << x << " " << y << " " << halfLength << " " << halfWidth << endl;

    // GL3 accepts only Triangles. Quads are not supported
  GLfloat vertex_buffer_data [] = {
    x - halfWidth, y + halfLength, z, // vertex 1
    x - halfWidth, y - halfLength, z, // vertex 2
    x + halfWidth, y - halfLength, z, // vertex 3

    x + halfWidth, y - halfLength, z, // vertex 3
    x + halfWidth, y + halfLength, z, // vertex 4
    x - halfWidth, y + halfLength, z // vertex 1
  };

  GLfloat color_buffer_data [] = {
    red ,blue, green, // color 1
    red ,blue, green, // color 2
    red ,blue, green, // color 3

    red ,blue, green, // color 3
    red ,blue, green, // color 4
    red ,blue, green  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  if(fill_mode)
    return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  else
    return create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_LINE);
}

VAO* drawBeak(GLfloat x, GLfloat y, GLfloat z, GLfloat size)
{
  GLfloat beakSize = size / 2;
  GLfloat vertex_buffer_data [] = {
    x, y + beakSize, z,
    x + size + (beakSize * 2), y, z,
    x, y - beakSize , z  
  };
  GLfloat color_buffer_data [] = {
    1, 0.5, 0,
    1, 0.5, 0,
    1, 0.5, 0
  };
  return create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}

int numOfIce = 0;
VAO *triangle, *rectangle, *bird, *ground, *birdFace, *birdBeak, *birdEyeIris, *birdEyeSclera, *canonWheel, *canonTunnel, *iceBricks[100], *iceBricksOutline[100];

//Pupil and Sclera are the colored part in eye, I assume that bird has no  pupil

// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1.2,-1,0, // vertex 1
    1.2,-1,0, // vertex 2
    1.2, 1,0, // vertex 3

    1.2, 1,0, // vertex 3
    -1.2, 1,0, // vertex 4
    -1.2,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0.3,0.3,0.3, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBirdEye(GLfloat radius, GLfloat x, GLfloat y, GLfloat z)
{
  GLfloat irisRadius = radius/3;
  GLfloat scleraRadius = irisRadius / 2;
  GLfloat theAngle = M_PI/6;
  GLfloat xIris = x + (radius - irisRadius) * cos(theAngle);
  GLfloat yIris = y + (radius - irisRadius) * sin(theAngle);
  GLfloat zIris = z;
  birdEyeIris = drawCircle(xIris, yIris, zIris, irisRadius, 360, 0, 0, 0) ;
  GLfloat xSclera = xIris + (irisRadius - scleraRadius) * cos(theAngle);
  GLfloat ySclera = yIris + (irisRadius - scleraRadius) * sin(theAngle);
  GLfloat zSclera = zIris;
  birdEyeSclera = drawCircle(xSclera, ySclera, zSclera, scleraRadius, 360, 1, 1, 1);
}


void createBird(GLfloat size, GLfloat red, GLfloat blue, GLfloat green, int order, int type)
{
  GLfloat radius = size;
  GLfloat x = (float)GROUND_HEIGHT + radius; //Illogical but just for sake :'(
  GLfloat y = (float)GROUND_HEIGHT + radius;
  GLfloat z = 0;
  GLint numberOfSides = 360;
  birdFace = drawCircle(x, y, z, radius, numberOfSides, red, blue, green);
  birdBeak = drawBeak(x, y, z, size);
  createBirdEye(size, x, y, z);
}


void createGround()
{
    // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    0,0,0, // vertex 1
    SCREEN_WIDTH,0,0, // vertex 2
    SCREEN_WIDTH, GROUND_HEIGHT,0, // vertex 3

    SCREEN_WIDTH, GROUND_HEIGHT,0, // vertex 3
    0, GROUND_HEIGHT, 0, // vertex 4
    0,0,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0,1,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,1,0, // color 4
    0,1,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  ground = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL); 
}

void createCanon()
{
  GLfloat radius = (float)(CANON_WHEEL_CENTERY - GROUND_HEIGHT);
  GLfloat padding = 5.0f;
  //drawCircle(GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides, GLfloat red, GLfloat blue, GLfloat green)
  canonWheel = drawCircle(CANON_WHEEL_CENTERX, CANON_WHEEL_CENTERY, 0, radius, 360, 0.3, 0.3, 0.3);
  static const GLfloat vertex_buffer_data [] = {
    CANON_WHEEL_CENTERX,CANON_WHEEL_CENTERY,0, // vertex 1
    CANON_WHEEL_CENTERX,CANON_WHEEL_CENTERY + radius - padding,0, // vertex 2
    CANON_WHEEL_CENTERX + CANON_TUNNEL_LENGTH,CANON_WHEEL_CENTERY + radius - padding,0, // vertex 3

    CANON_WHEEL_CENTERX + CANON_TUNNEL_LENGTH,CANON_WHEEL_CENTERY + radius -padding,0, // vertex 3
    CANON_WHEEL_CENTERX + CANON_TUNNEL_LENGTH,CANON_WHEEL_CENTERY,0, // vertex 4
    CANON_WHEEL_CENTERX ,CANON_WHEEL_CENTERY,0// vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    0.5,0.5,0.5, // color 1
    0.5,0.5,0.5, // color 2
    0.5,0.5,0.5, // color 3

    0.5,0.5,0.5, // color 3
    0.5,0.5,0.5, // color 4
    0.5,0.5,0.5  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  canonTunnel = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createIcebricksSquare(GLint sizeOfMesh, GLfloat depth, GLfloat startX)
{
  float vertex_matrix[sizeOfMesh][sizeOfMesh];
  float x =  (float)(startX + (OBSTACLE_ICE_SIZE/2));
  float y = (float)(GROUND_HEIGHT + (OBSTACLE_ICE_SIZE/2));
  float padding = 2.0f;
  for (int i = 0; i < sizeOfMesh; i++)
  {
    for (int j = 0; j < sizeOfMesh; j++)
    {
      if(i < depth || i > sizeOfMesh - (depth + 1) || j < depth || j > sizeOfMesh - (depth + 1))
      {
        iceBricksOutline[numOfIce] = drawRectangle(x + (float)(i * OBSTACLE_ICE_SIZE), y + (float)(j * OBSTACLE_ICE_SIZE), 0.0f, OBSTACLE_ICE_SIZE/2, OBSTACLE_ICE_SIZE/2, 0.65f, 0.94f, 0.95f, false);
        iceBricks[numOfIce++] = drawRectangle(x + (float)(i * OBSTACLE_ICE_SIZE), y + (float)(j * OBSTACLE_ICE_SIZE), 0.0f, (OBSTACLE_ICE_SIZE/2) - padding, (OBSTACLE_ICE_SIZE/2) - padding, 0.65f, 0.94f, 0.95f, true);
      }
    }
  }
}


float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */

  glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(triangle);

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle = glm::translate (glm::vec3(2.0f, 0.0f, 0.0f));        // glTranslatef
  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(rectangle);
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateBird = glm::translate (glm::vec3(0, 0, 0));        // glTranslatef
  glm::mat4 rotateBird = glm::rotate(0.0f, glm::vec3(0,0,1));
  Matrices.model *= translateBird * rotateBird;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(birdBeak);
  draw3DObject(birdFace);
  draw3DObject(birdEyeIris);
  draw3DObject(birdEyeSclera);
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateGround = glm::translate (glm::vec3(0, 0, 0));        // glTranslatef
  glm::mat4 rotateGround = glm::rotate(0.0f, glm::vec3(0,0,1));
  Matrices.model *= translateGround * rotateGround;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  draw3DObject(ground);

  canon_tunnel_angle += canon_tunnel_rotation;
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateCanon = glm::translate (glm::vec3(CANON_WHEEL_CENTERX, CANON_WHEEL_CENTERY, 0));        // glTranslatef
  glm::mat4 rotateCanon = glm::rotate(canon_tunnel_angle, glm::vec3(0, 0, 1));
  Matrices.model *= translateCanon * rotateCanon;
  translateCanon = glm::translate (glm::vec3(-1*CANON_WHEEL_CENTERX, -1*CANON_WHEEL_CENTERY, 0));        // glTranslatef
  rotateCanon = glm::rotate(0.0f, glm::vec3(0, 0, 1));
  Matrices.model *= translateCanon * rotateCanon; 
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  draw3DObject(canonTunnel);
  draw3DObject(canonWheel);

  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateIce = glm::translate (glm::vec3(0, 0, 0));        // glTranslatef
  glm::mat4 rotateIce = glm::rotate(0.0f, glm::vec3(0, 0, 1));
  Matrices.model *= translateIce* rotateIce; 
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  for (int i = 0; i < numOfIce; i++)
  {
    draw3DObject(iceBricksOutline[i]);
    draw3DObject(iceBricks[i]);
  }


  // Increment angles 
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();
	createBird(10.0f, 1, 0, 0, 1, 0);
  createGround();
  createCanon();
  createIcebricksSquare(3, 1, OBSTACLE_STARTSX);
  // createPiggy();
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.0f, 0.4f, 0.8f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{

  GLFWwindow* window = initGLFW(SCREEN_WIDTH, SCREEN_HEIGHT);

	initGL (window, SCREEN_WIDTH, SCREEN_HEIGHT);

  double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
