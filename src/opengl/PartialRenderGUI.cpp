#include <GL/glut.h>
#include <thread>
#include <glm/gtx/euler_angles.hpp>
#include "header/Params.h"
#include "header/VideoStitch.h"
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

int WINDOW_SIZE_WIDTH = 400;
int WINDOW_SIZE_HEIGHT = 400;

float gPitch = 0.f;
float gYaw = 0.f;

GLdouble theta = -M_PI/2, phi = M_PI / 2;
GLdouble eye_x = 0.0, eye_y = 3, eye_z = -3.5,
         center_x = eye_x + sin(phi) * cos(theta), center_y = eye_y + cos(phi), center_z = 4*sin(phi) * sin(theta),
         up_x = 0.0, up_y = 1.0, up_z = 0.0;

void handleKeypress(unsigned char key, int x, int y) {
    switch (key) {
        case 27: //Escape key
            exit(0);
            break;
        case 97: //a
            gYaw -= 10.0f;
            break;
        case 100: //d
            gYaw += 10.0f;
            break;
        case 119: //w
            gPitch -= 10.0f;
            gPitch = gPitch < -90.f ? -90.f : gPitch;
            break;
        case 115: //s
            gPitch += 10.0f;
            gPitch = gPitch > 90.f ? 90.f : gPitch;
            break;
    }
}

GLuint initTexture() {
    int w = OUTPUT_PANO_WIDTH;
    int h = OUTPUT_PANO_HEIGHT;
    GLuint textureId;
    glGenTextures(1, &textureId); //Make room for our texture
    glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit
    //Map the image to the texture
    glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
                 0,                            //0 for now
                 GL_RGB,                       //Format OpenGL uses for image
                 w, h,  //Width and height
                 0,                            //The border of the image
                 GL_BGR, //GL_RGB, because pixels are stored in RGB format
                 GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
                                   //as unsigned numbers
                 nullptr);               //The actual pixel data
    return textureId; //Returns the id of the texture
}

GLuint gTextureId; //The id of the textur
GLUquadric *gQuad;
Mat gLatestImg;

void initRendering() {
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_MULTISAMPLE);
    gQuad = gluNewQuadric();
    gLatestImg = imread("tmp.png");
    gTextureId = initTexture();
}

void handleResize(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 1.0, 200.0);
}

void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(eye_x, eye_y, eye_z, center_x, center_y, center_z, up_x, up_y, up_z);

    glRotatef(90, 1.0f, 0.0f, 0.0f);
    
    glTranslatef(0.0f, 0.0f, 0.f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, OUTPUT_PANO_WIDTH, OUTPUT_PANO_HEIGHT, GL_BGR, GL_UNSIGNED_BYTE, gLatestImg.data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    gluQuadricTexture(gQuad, GLU_TRUE);
    gluSphere(gQuad, 128, 20, 20);

    glutSwapBuffers();
}


void update(int value) {
    //gPitch += 2.f;
    glutPostRedisplay();
    glutTimerFunc(25, update, 0);
}

void updateImage(Mat img) {
    //cv::flip(img, gLatestImg, 0);
    gLatestImg = img;
}

void updateRenderRegion(float& centerU, float& centerV, float& range) {
}

void launchVideoStitch(int argc, char** argv) {
    std::unique_ptr<VideoStitcher> vs ( new VideoStitcher(argc, argv) );
    vs->registerCallbackFunc ( updateImage );
    vs->registerUpdaterFunc ( updateRenderRegion );
    vs->doRealTimeStitching(argc, argv);
    vs.release();
}

int start_x, start_y;

void mouseButton(int btn, int state, int x, int y) {
    if(state == GLUT_DOWN) {
        start_x = x;
        start_y = y;
    }
}

void mouseMove(int x, int y) {
    theta += 2 * static_cast<double> (x - start_x) / WINDOW_SIZE_WIDTH;
    if(theta > 2 * M_PI) theta -= 2 * M_PI;
    if(theta < -2 * M_PI) theta += 2 * M_PI;
    GLdouble tmp = phi;
    phi += 2 * static_cast<double> (y - start_y) / WINDOW_SIZE_HEIGHT;
    if(phi > 0 && phi < M_PI)
        center_y = eye_y + cos(phi);
    else
        phi = tmp;
    center_x = eye_x + sin(phi) * cos(theta);
    center_z = eye_z + sin(phi) * sin(theta);
    start_x = x;
    start_y = y;
}
int main(int argc, char** argv) {
    signal(SIGSEGV, segFaultHandler);

    thread mThread(launchVideoStitch, argc, argv);

    /** Initialize GLUT */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(WINDOW_SIZE_WIDTH, WINDOW_SIZE_HEIGHT);
    glutCreateWindow("Video Stitcher Viewer");

    initRendering();
    
    glutTimerFunc(25, update, 0);
    glutDisplayFunc(drawScene);
    glutKeyboardFunc(handleKeypress);
    glutReshapeFunc(handleResize);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMove);
    glutMainLoop();

    // Wait for thread to join
    mThread.join();

    return 0;
}