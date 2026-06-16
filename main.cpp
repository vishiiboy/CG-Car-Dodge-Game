#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <sstream>

// --- MEMBER 2: Player car position, size, and movement ----------------------
const float PLAYER_START_X = 0.0f;
const float PLAYER_START_Y = -0.75f;
const float PLAYER_MIN_X = -0.36f;
const float PLAYER_MAX_X = 0.36f;
const float PLAYER_MOVE_SPEED = 0.045f;

float playerX = PLAYER_START_X;
float playerY = PLAYER_START_Y;
float playerWidth = 0.18f;
float playerHeight = 0.30f;
int playerMoveDirection = 0;
bool moveLeftPressed = false;
bool moveRightPressed = false;
// ----------------------------------------------------------------------------

// --- MEMBER 3: Enemy struct and array ---------------------------------------
struct Enemy {
    float x, y;
    float width, height;
    float speed;
    int type;       // 0 = car, 1 = truck, 2 = pothole hazard
    float angle;    // current rotation angle (used by hazard type)
    bool active;
};

const int MAX_ENEMIES = 4;
Enemy enemies[MAX_ENEMIES];
// ----------------------------------------------------------------------------

// --- MEMBER 5: Score and Level Variables ------------------------------------
int score = 0;
int level = 1;
int nextLevelScore = 5;
// ----------------------------------------------------------------------------

bool gameOver = false;
bool gameStarted = false;
float laneLineOffset = 0.0f;
const float ROAD_SPEED = 0.02f;   // speed the road scrolls down; potholes match this

// --- MEMBER 4: Visual feedback variables ------------------------------------
float potholeFlashTimer = 0.0f;   // For yellow flash when hitting pothole

void drawText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

void drawRectangle(float x, float y, float width, float height) {
    glBegin(GL_QUADS);
        glVertex2f(x - width / 2, y - height / 2);
        glVertex2f(x + width / 2, y - height / 2);
        glVertex2f(x + width / 2, y + height / 2);
        glVertex2f(x - width / 2, y + height / 2);
    glEnd();
}

// --- MEMBER 2: Keep player movement inside the road -------------------------
void clampPlayerToRoad() {
    if (playerX < PLAYER_MIN_X) playerX = PLAYER_MIN_X;
    if (playerX > PLAYER_MAX_X) playerX = PLAYER_MAX_X;
}

void resetPlayer() {
    playerX = PLAYER_START_X;
    playerY = PLAYER_START_Y;
    playerMoveDirection = 0;
    moveLeftPressed = false;
    moveRightPressed = false;
}
// ----------------------------------------------------------------------------

void drawStartMenu() {
    glColor3f(0.0f, 0.45f, 0.0f);
    drawRectangle(0.0f, 0.0f, 2.0f, 2.0f);

    glColor3f(0.1f, 0.1f, 0.1f);
    drawRectangle(0.0f, 0.0f, 1.2f, 1.4f);

    glColor3f(1.0f, 1.0f, 0.0f);
    drawText(-0.33f, 0.45f, "CAR DODGE GAME");

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(-0.35f, 0.20f, "Press ENTER to Start");

    drawText(-0.38f, -0.05f, "Controls:");
    drawText(-0.38f, -0.18f, "A / Left Arrow  - Move Left");
    drawText(-0.38f, -0.31f, "D / Right Arrow - Move Right");
    drawText(-0.38f, -0.44f, "R - Restart");
    drawText(-0.38f, -0.57f, "ESC - Exit");
}

void drawRoad() {
    // Grass background
    glColor3f(0.0f, 0.45f, 0.0f);
    drawRectangle(0.0f, 0.0f, 2.0f, 2.0f);

    // Road (widened for 4 lanes)
    glColor3f(0.12f, 0.12f, 0.12f);
    drawRectangle(0.0f, 0.0f, 1.20f, 2.0f);

    // Road side borders
    glColor3f(1.0f, 1.0f, 1.0f);
    drawRectangle(-0.60f, 0.0f, 0.03f, 2.0f);
    drawRectangle(0.60f, 0.0f, 0.03f, 2.0f);

    // Yellow side strips
    glColor3f(1.0f, 0.85f, 0.0f);
    drawRectangle(-0.55f, 0.0f, 0.018f, 2.0f);
    drawRectangle(0.55f, 0.0f, 0.018f, 2.0f);

    // Moving dashed lane dividers (3 dividers between 4 lanes)
    glColor3f(1.0f, 1.0f, 1.0f);

    float dividers[] = { -0.24f, 0.0f, 0.24f };
    float startY = -1.2f + laneLineOffset;

    for (int d = 0; d < 3; d++) {
        for (int i = 0; i < 7; i++) {
            float y = startY + i * 0.4f;
            drawRectangle(dividers[d], y, 0.030f, 0.22f);
        }
    }

    // Simple trees on grass area
    glColor3f(0.35f, 0.18f, 0.05f);
    drawRectangle(-0.82f, 0.65f, 0.04f, 0.18f);
    drawRectangle(0.82f, 0.30f, 0.04f, 0.18f);
    drawRectangle(-0.82f, -0.25f, 0.04f, 0.18f);
    drawRectangle(0.82f, -0.70f, 0.04f, 0.18f);

    glColor3f(0.0f, 0.65f, 0.0f);
    drawRectangle(-0.82f, 0.78f, 0.16f, 0.16f);
    drawRectangle(0.82f, 0.43f, 0.16f, 0.16f);
    drawRectangle(-0.82f, -0.12f, 0.16f, 0.16f);
    drawRectangle(0.82f, -0.57f, 0.16f, 0.16f);
}

// --- MEMBER 2: Draw detailed player car facing upward -----------------------
void drawPlayerCar(float x, float y) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(-playerMoveDirection * 3.0f, 0.0f, 0.0f, 1.0f);

    // Ground shadow
    glColor3f(0.03f, 0.03f, 0.03f);
    drawRectangle(0.0f, -0.02f, 0.20f, 0.31f);

    // Rear spoiler
    glColor3f(0.04f, 0.04f, 0.10f);
    drawRectangle(0.0f, -0.16f, 0.23f, 0.03f);
    drawRectangle(-0.09f, -0.135f, 0.02f, 0.045f);
    drawRectangle(0.09f, -0.135f, 0.02f, 0.045f);

    // Tyres
    glColor3f(0.0f, 0.0f, 0.0f);
    drawRectangle(-0.10f, 0.09f, 0.045f, 0.105f);
    drawRectangle(0.10f, 0.09f, 0.045f, 0.105f);
    drawRectangle(-0.10f, -0.09f, 0.045f, 0.105f);
    drawRectangle(0.10f, -0.09f, 0.045f, 0.105f);

    // Hubcaps
    glColor3f(0.72f, 0.72f, 0.78f);
    drawRectangle(-0.10f, 0.09f, 0.018f, 0.04f);
    drawRectangle(0.10f, 0.09f, 0.018f, 0.04f);
    drawRectangle(-0.10f, -0.09f, 0.018f, 0.04f);
    drawRectangle(0.10f, -0.09f, 0.018f, 0.04f);

    // Main body
    glColor3f(0.10f, 0.40f, 1.0f);
    drawRectangle(0.0f, 0.0f, 0.17f, 0.30f);

    // Dark side panels
    glColor3f(0.05f, 0.24f, 0.70f);
    drawRectangle(-0.077f, 0.0f, 0.018f, 0.30f);
    drawRectangle(0.077f, 0.0f, 0.018f, 0.30f);

    // Hood and cabin
    glColor3f(0.25f, 0.55f, 1.0f);
    drawRectangle(0.0f, 0.11f, 0.14f, 0.06f);
    glColor3f(0.07f, 0.30f, 0.85f);
    drawRectangle(0.0f, -0.01f, 0.13f, 0.13f);

    // Windows
    glColor3f(0.62f, 0.90f, 1.0f);
    drawRectangle(0.0f, 0.055f, 0.11f, 0.05f);
    glColor3f(0.48f, 0.80f, 0.95f);
    drawRectangle(0.0f, -0.075f, 0.11f, 0.045f);

    // Racing stripes
    glColor3f(1.0f, 1.0f, 1.0f);
    drawRectangle(-0.028f, 0.115f, 0.014f, 0.05f);
    drawRectangle(0.028f, 0.115f, 0.014f, 0.05f);
    drawRectangle(-0.028f, -0.01f, 0.014f, 0.11f);
    drawRectangle(0.028f, -0.01f, 0.014f, 0.11f);

    // Lights
    glColor3f(1.0f, 1.0f, 0.65f);
    drawRectangle(-0.06f, 0.14f, 0.03f, 0.02f);
    drawRectangle(0.06f, 0.14f, 0.03f, 0.02f);
    glColor3f(1.0f, 0.12f, 0.12f);
    drawRectangle(-0.06f, -0.14f, 0.03f, 0.02f);
    drawRectangle(0.06f, -0.14f, 0.03f, 0.02f);

    glPopMatrix();
}
// ----------------------------------------------------------------------------

// --- MEMBER 3: Draw enemy car, truck, or pothole hazard ---------------------
void drawEnemy(float x, float y, int type, float angle) {
    if (type == 2) {
        // Pothole road hazard
        glPushMatrix();
        glTranslatef(x, y, 0.0f);

        glColor3f(0.22f, 0.22f, 0.22f);
        drawRectangle(0.0f, 0.0f, 0.24f, 0.24f);

        glPushMatrix();
        glRotatef(angle, 0.0f, 0.0f, 1.0f);
        glColor3f(0.32f, 0.29f, 0.24f);
        drawRectangle(0.0f, 0.105f, 0.022f, 0.022f);
        drawRectangle(0.0f, -0.105f, 0.022f, 0.022f);
        drawRectangle(0.105f, 0.0f, 0.022f, 0.022f);
        drawRectangle(-0.105f, 0.0f, 0.022f, 0.022f);
        glPopMatrix();

        glColor3f(0.07f, 0.07f, 0.07f);
        drawRectangle(0.0f, 0.0f, 0.18f, 0.18f);
        glColor3f(0.03f, 0.03f, 0.03f);
        drawRectangle(0.0f, -0.01f, 0.13f, 0.13f);
        glColor3f(0.0f, 0.0f, 0.0f);
        drawRectangle(0.0f, -0.02f, 0.08f, 0.08f);

        glColor3f(0.28f, 0.28f, 0.28f);
        drawRectangle(0.0f, 0.14f, 0.008f, 0.05f);
        drawRectangle(0.10f, -0.10f, 0.05f, 0.008f);
        drawRectangle(-0.11f, 0.07f, 0.04f, 0.008f);

        glPopMatrix();
    } else if (type == 1) {
        // Green delivery truck
        glColor3f(0.0f, 0.0f, 0.0f);
        drawRectangle(x - 0.10f, y + 0.12f, 0.04f, 0.08f);
        drawRectangle(x + 0.10f, y + 0.12f, 0.04f, 0.08f);
        drawRectangle(x - 0.10f, y + 0.00f, 0.04f, 0.08f);
        drawRectangle(x + 0.10f, y + 0.00f, 0.04f, 0.08f);
        drawRectangle(x - 0.10f, y - 0.12f, 0.04f, 0.08f);
        drawRectangle(x + 0.10f, y - 0.12f, 0.04f, 0.08f);

        glColor3f(0.10f, 0.42f, 0.10f);
        drawRectangle(x, y + 0.07f, 0.20f, 0.22f);

        glColor3f(0.06f, 0.28f, 0.06f);
        drawRectangle(x, y + 0.135f, 0.20f, 0.012f);
        drawRectangle(x, y + 0.070f, 0.20f, 0.012f);
        drawRectangle(x, y + 0.005f, 0.20f, 0.012f);

        glColor3f(0.16f, 0.56f, 0.16f);
        drawRectangle(x, y - 0.13f, 0.19f, 0.12f);

        glColor3f(0.50f, 0.85f, 0.90f);
        drawRectangle(x, y - 0.155f, 0.14f, 0.05f);

        glColor3f(1.0f, 1.0f, 0.70f);
        drawRectangle(x - 0.07f, y - 0.185f, 0.03f, 0.02f);
        drawRectangle(x + 0.07f, y - 0.185f, 0.03f, 0.02f);
        glColor3f(0.30f, 0.30f, 0.30f);
        drawRectangle(x, y - 0.195f, 0.17f, 0.018f);
    } else {
        // Red enemy car
        glColor3f(0.0f, 0.0f, 0.0f);
        drawRectangle(x - 0.085f, y + 0.09f, 0.04f, 0.10f);
        drawRectangle(x + 0.085f, y + 0.09f, 0.04f, 0.10f);
        drawRectangle(x - 0.085f, y - 0.09f, 0.04f, 0.10f);
        drawRectangle(x + 0.085f, y - 0.09f, 0.04f, 0.10f);

        glColor3f(0.62f, 0.62f, 0.66f);
        drawRectangle(x - 0.085f, y + 0.09f, 0.016f, 0.04f);
        drawRectangle(x + 0.085f, y + 0.09f, 0.016f, 0.04f);
        drawRectangle(x - 0.085f, y - 0.09f, 0.016f, 0.04f);
        drawRectangle(x + 0.085f, y - 0.09f, 0.016f, 0.04f);

        glColor3f(0.88f, 0.10f, 0.10f);
        drawRectangle(x, y, 0.17f, 0.28f);

        glColor3f(0.58f, 0.05f, 0.05f);
        drawRectangle(x - 0.077f, y, 0.018f, 0.28f);
        drawRectangle(x + 0.077f, y, 0.018f, 0.28f);

        glColor3f(1.0f, 0.22f, 0.22f);
        drawRectangle(x, y - 0.10f, 0.14f, 0.06f);

        glColor3f(0.66f, 0.06f, 0.06f);
        drawRectangle(x, y + 0.01f, 0.13f, 0.12f);

        glColor3f(0.30f, 0.42f, 0.52f);
        drawRectangle(x, y - 0.05f, 0.11f, 0.05f);
        drawRectangle(x, y + 0.075f, 0.11f, 0.045f);

        glColor3f(1.0f, 1.0f, 0.70f);
        drawRectangle(x - 0.055f, y - 0.135f, 0.03f, 0.02f);
        drawRectangle(x + 0.055f, y - 0.135f, 0.03f, 0.02f);

        glColor3f(0.45f, 0.0f, 0.0f);
        drawRectangle(x - 0.055f, y + 0.135f, 0.03f, 0.02f);
        drawRectangle(x + 0.055f, y + 0.135f, 0.03f, 0.02f);
    }
}
// ----------------------------------------------------------------------------

// --- MEMBER 3: Reset a single enemy at index i ------------------------------
void resetEnemy(int i) {
    float lanePositions[] = { -0.36f, -0.12f, 0.12f, 0.36f };
    const int LANE_COUNT = 4;
    float spawnY = 1.1f + i * 0.65f;

    int lane;
    bool laneOk;
    int attempts = 0;
    do {
        lane = rand() % LANE_COUNT;
        laneOk = true;
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (j != i && enemies[j].active &&
                enemies[j].x == lanePositions[lane] &&
                fabs(enemies[j].y - spawnY) < 0.55f) {
                laneOk = false;
                break;
            }
        }
        attempts++;
    } while (!laneOk && attempts < 12);

    enemies[i].x     = lanePositions[lane];
    enemies[i].y     = spawnY;
    enemies[i].type  = rand() % 3;
    enemies[i].angle = 0.0f;
    if (enemies[i].type == 1) {
        enemies[i].width  = 0.20f;
        enemies[i].height = 0.36f;
    } else if (enemies[i].type == 2) {
        enemies[i].width  = 0.22f;
        enemies[i].height = 0.22f;
    } else {
        enemies[i].width  = 0.18f;
        enemies[i].height = 0.28f;
    }
    if (enemies[i].type == 2) {
        enemies[i].speed = ROAD_SPEED;
    } else {
        enemies[i].speed = 0.015f + (rand() % 5) * 0.002f;
    }
    enemies[i].active = true;
}

void initEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        resetEnemy(i);
    }
}

// --- MEMBER 4: Collision detection (Cars/Trucks = Game Over, Potholes = Penalty) ---
// Returns: 0 = no collision, 1 = fatal (car/truck), 2 = pothole (penalty)
int checkCollisionType() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active &&
            playerX - playerWidth / 2  < enemies[i].x + enemies[i].width / 2 &&
            playerX + playerWidth / 2  > enemies[i].x - enemies[i].width / 2 &&
            playerY - playerHeight / 2 < enemies[i].y + enemies[i].height / 2 &&
            playerY + playerHeight / 2 > enemies[i].y - enemies[i].height / 2) {
            
            // Different response based on enemy type
            if (enemies[i].type == 2) {
                return 2;  // Pothole - penalty only
            } else {
                return 1;  // Car or Truck - game over
            }
        }
    }
    return 0;  // No collision
}

// Original function name kept for compatibility with other code
bool checkCollision() {
    return (checkCollisionType() == 1);
}
// ----------------------------------------------------------------------------

void updateGame(int value) {
    if (gameStarted && !gameOver) {
        // Smooth player movement
        playerMoveDirection = 0;
        if (moveLeftPressed && !moveRightPressed) {
            playerMoveDirection = -1;
            playerX -= PLAYER_MOVE_SPEED;
        } else if (moveRightPressed && !moveLeftPressed) {
            playerMoveDirection = 1;
            playerX += PLAYER_MOVE_SPEED;
        }
        clampPlayerToRoad();

        laneLineOffset -= ROAD_SPEED;
        if (laneLineOffset < -0.4f) {
            laneLineOffset = 0.0f;
        }

        // Move all enemies
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                float newY = enemies[i].y - enemies[i].speed;

                for (int j = 0; j < MAX_ENEMIES; j++) {
                    if (j != i && enemies[j].active &&
                        enemies[j].x == enemies[i].x &&
                        enemies[j].y < enemies[i].y) {
                        float minGap = (enemies[i].height + enemies[j].height) / 2.0f + 0.05f;
                        if (newY - enemies[j].y < minGap) {
                            newY = enemies[j].y + minGap;
                        }
                    }
                }
                enemies[i].y = newY;

                if (enemies[i].type == 2) {
                    enemies[i].angle += 1.5f;
                    if (enemies[i].angle >= 360.0f) enemies[i].angle -= 360.0f;
                }

                // SCORE UPDATE - When enemy goes off screen (bottom)
                if (enemies[i].y < -1.1f) {
                    // Only add score for cars and trucks (type 0 and 1), not potholes
                    if (enemies[i].type != 2) {
                        score++;
                        
                        // Level progression
                        if (score >= nextLevelScore) {
                            level++;
                            nextLevelScore += 5;
                        }
                        
                        // Increase speed based on level
                        for (int j = 0; j < MAX_ENEMIES; j++) {
                            if (enemies[j].type != 2) {
                                enemies[j].speed += (0.0005f * level);
                            }
                        }
                    }
                    
                    resetEnemy(i);
                }
            }
        }

        // --- MEMBER 4: Handle different collision types ---
        int collisionResult = checkCollisionType();
        if (collisionResult == 1) {
            // Car or truck - Game Over
            gameOver = true;
        }
        else if (collisionResult == 2) {
            // Pothole - Deduct points but continue game
            score = (score >= 10) ? score - 10 : 0;
            
            // Trigger visual feedback (yellow flash)
            potholeFlashTimer = 8.0f;
            
            // Deactivate the pothole that was hit
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active && enemies[i].type == 2) {
                    if (playerX - playerWidth/2 < enemies[i].x + enemies[i].width/2 &&
                        playerX + playerWidth/2 > enemies[i].x - enemies[i].width/2 &&
                        playerY - playerHeight/2 < enemies[i].y + enemies[i].height/2 &&
                        playerY + playerHeight/2 > enemies[i].y - enemies[i].height/2) {
                        enemies[i].active = false;  // Remove pothole after hit
                        break;
                    }
                }
            }
        }
        
        // Update pothole flash timer
        if (potholeFlashTimer > 0) {
            potholeFlashTimer -= 1.0f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, updateGame, 0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (!gameStarted) {
        drawStartMenu();
        glFlush();
        return;
    }

    drawRoad();
    drawPlayerCar(playerX, playerY);

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            drawEnemy(enemies[i].x, enemies[i].y, enemies[i].type, enemies[i].angle);
        }
    }

    // --- MEMBER 4: Pothole Flash Effect (Yellow flash when hitting pothole) ---
    if (potholeFlashTimer > 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        float alpha = potholeFlashTimer / 8.0f * 0.4f;
        glColor4f(1.0f, 1.0f, 0.0f, alpha);  // Yellow flash
        drawRectangle(0.0f, 0.0f, 2.0f, 2.0f);
        glDisable(GL_BLEND);
    }

    // --- MEMBER 5: Polished UI Panel -----------------------------------
    glColor3f(0.1f, 0.1f, 0.1f);
    drawRectangle(-0.82f, 0.85f, 0.35f, 0.20f);

    glColor3f(1.0f, 1.0f, 0.0f);
    std::stringstream ssScore;
    ssScore << "Score: " << score;
    drawText(-0.95f, 0.89f, ssScore.str());

    glColor3f(0.0f, 1.0f, 1.0f);
    std::stringstream ssLevel;
    ssLevel << "Level: " << level;
    drawText(-0.95f, 0.80f, ssLevel.str());
    
    // --- MEMBER 4: Add collision info to UI ---
    glColor3f(0.7f, 0.7f, 0.7f);
    drawText(-0.95f, 0.71f, "Avoid RED cars & trucks");
    drawText(-0.95f, 0.63f, "Potholes = -10 points");
    // -------------------------------------------------------------------

    if (gameOver) {
        // --- MEMBER 5: Polished Game Over Screen -----------------------
        glColor3f(0.0f, 0.0f, 0.0f);
        drawRectangle(0.0f, 0.0f, 1.0f, 0.6f); 

        glColor3f(1.0f, 0.0f, 0.0f);
        drawText(-0.25f, 0.15f, "GAME OVER");

        glColor3f(1.0f, 1.0f, 0.0f);
        std::stringstream ssFinal;
        ssFinal << "Final Score: " << score << " (Level " << level << ")";
        drawText(-0.35f, -0.02f, ssFinal.str());

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(-0.35f, -0.15f, "Press R to Restart");
        // ---------------------------------------------------------------
    }

    glFlush();
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 13) {  // ENTER key
        gameStarted = true;
        glutPostRedisplay();
        return;
    }

    if (key == 'a' || key == 'A') {
        moveLeftPressed = true;
    }

    if (key == 'd' || key == 'D') {
        moveRightPressed = true;
    }

    if (key == 'r' || key == 'R') {
        gameOver = false;
        score = 0;
        
        // --- MEMBER 5: Level reset on restart ---
        level = 1;              
        nextLevelScore = 5;
        
        // --- MEMBER 4: Reset flash timer ---
        potholeFlashTimer = 0.0f;
        // ----------------------------------------
        
        gameStarted = true;
        resetPlayer();
        initEnemies();
    }

    if (key == 27) {  // ESC key
        exit(0);
    }

    clampPlayerToRoad();
    glutPostRedisplay();
}

void keyboardUp(unsigned char key, int x, int y) {
    if (key == 'a' || key == 'A') {
        moveLeftPressed = false;
    }

    if (key == 'd' || key == 'D') {
        moveRightPressed = false;
    }

    if (!moveLeftPressed && !moveRightPressed) {
        playerMoveDirection = 0;
    }

    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT) {
        moveLeftPressed = true;
    }

    if (key == GLUT_KEY_RIGHT) {
        moveRightPressed = true;
    }

    clampPlayerToRoad();
    glutPostRedisplay();
}

void specialKeysUp(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT) {
        moveLeftPressed = false;
    }

    if (key == GLUT_KEY_RIGHT) {
        moveRightPressed = false;
    }

    if (!moveLeftPressed && !moveRightPressed) {
        playerMoveDirection = 0;
    }

    glutPostRedisplay();
}

void init() {
    glClearColor(0.0f, 0.45f, 0.0f, 1.0f);
    glEnable(GL_BLEND);  // Enable transparency for flash effects
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    srand(time(0));
    resetPlayer();
    initEnemies();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 700);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Car Dodge Game - Member 4 Collision Detection");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeys);
    glutSpecialUpFunc(specialKeysUp);
    glutIgnoreKeyRepeat(1);
    glutTimerFunc(16, updateGame, 0);

    glutMainLoop();

    return 0;
}