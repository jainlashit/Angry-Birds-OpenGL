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

typedef struct Obstacle{
  int index;
  int x;
  int y;
  int replacing;
  bool isPiggy;
  bool toReplace;
}Obstacle;

GLuint programID;

Obstacle all[10][10];
VAO *ground;
VAO *bird[10], *birdFace[10], *birdBeak[10], *birdEyeIris[10], *birdEyeSclera[10];
VAO *canonWheel, *canonTunnel, *PowerPanelFill, *PowerPanelOut;
VAO *iceBricks[30], *iceBricksOutline[30], *iceBreakLines[30];
VAO *piggyFace[30], *piggyLeftEyeIris[10], *piggyRightEyeIris[10], *piggyLeftEyeSclera[10], *piggyRightEyeSclera[10], *piggyNose[10];

float screen_height = SCREEN_HEIGHT;
float screen_width = SCREEN_WIDTH;
int numOfIce = 0, numOfPiggy = 0, numOfBirds = 0;
int birdStatus[10] = {0}, birdType[10]; 
float birdDisplaceX[10] , birdDisplaceY[10], birdSize[10];
int iceBroken[30] = {0};
float iceBoundingCircle[30], iceX[30], iceY[30], iceTranslate[10] = {0};
int piggyHurt[10] = {0};
float piggyRadius[10] = {0}, piggyX[10], piggyY[10], piggyTranslate[10] = {0};
float canonMomentum = 100.0f;
float canon_tunnel_rotation = 0;
float canon_tunnel_angle = 0;





/*Physics Engine related*/
float phy_ux, phy_uy, phy_vy, phy_time = 0.0f, phy_x[10], phy_y[10], phy_angle, bird_storeX[10] = {0}, bird_storeY[10] = {0};
int phy_index;
bool phy_start = false;