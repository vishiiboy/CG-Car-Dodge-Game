// MEMBER 4: COLLISION DETECTION AND GAME OVER
// This file is meant to be used WITH main.cpp, not compiled alone

#include <GL/glut.h>
#include <string>

using namespace std;

// These variables are defined in main.cpp
extern float playerX;
extern float playerY;
extern float playerWidth;
extern float playerHeight;
extern float enemyX;
extern float enemyY;
extern float enemyWidth;
extern float enemyHeight;

bool gameOver = false;

bool checkCollision() {
    return playerX - playerWidth/2 < enemyX + enemyWidth/2 &&
           playerX + playerWidth/2 > enemyX - enemyWidth/2 &&
           playerY - playerHeight/2 < enemyY + enemyHeight/2 &&
           playerY + playerHeight/2 > enemyY - enemyHeight/2;
}

void drawGameOverScreen() {
    glColor3f(1.0f, 0.0f, 0.0f);
    glRasterPos2f(-0.25f, 0.1f);
    string gameOverText = "GAME OVER";
    for (char c : gameOverText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(-0.35f, -0.05f);
    string restartText = "Press R to Restart";
    for (char c : restartText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}
