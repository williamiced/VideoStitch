#include <GL/glut.h>
#include <thread>
#include "header/Params.h"
#include "header/VideoStitch.h"
#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

void handleKeypress(unsigned char key, int x, int y) {
    switch (key) {
        case 27: //Escape key
            exit(0);
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
float gRotateAngle;
Mat gLatestImg;

void initRendering() {
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    gQuad = gluNewQuadric();
    gLatestImg = Mat::zeros(OUTPUT_PANO_HEIGHT, OUTPUT_PANO_WIDTH, CV_8UC3);
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

    glTranslatef(0.0f, 1.0f, 0.f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, OUTPUT_PANO_WIDTH, OUTPUT_PANO_HEIGHT, GL_BGR, GL_UNSIGNED_BYTE, gLatestImg.data);

    //Bottom
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glRotatef(90,1.0f,0.0f,0.0f);
    glRotatef(gRotateAngle,0.0f,0.0f,1.0f);
    gluQuadricTexture(gQuad,1);
    gluSphere(gQuad,2,20,20);

    glutSwapBuffers();
}

void update(int value) {
    gRotateAngle+=2.0f;
    if (gRotateAngle>360.f) {
        gRotateAngle-=360.f;
    }
    glutPostRedisplay();
    glutTimerFunc(25,update,0);
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

int main(int argc, char** argv) {
    thread mThread(launchVideoStitch, argc, argv);

    /** Initialize GLUT */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Video Stitcher Viewer");

    initRendering();
    
    glutTimerFunc(25,update,0);
    glutDisplayFunc(drawScene);
    glutKeyboardFunc(handleKeypress);
    glutReshapeFunc(handleResize);
    glutMainLoop();

    // Wait for thread to join
    mThread.join();

    return 0;
}