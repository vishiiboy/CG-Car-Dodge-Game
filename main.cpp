#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <sstream>

float playerX = 0.0f;
float playerY = -0.75f;
float playerWidth = 0.18f;
float playerHeight = 0.28f;

// --- MEMBER 3: Enemy struct and array ---------------------------------------
struct Enemy {
    float x, y;
    float width, height;
    float speed;
    int type;       // 0 = car, 1 = truck, 2 = rotating hazard sign
    float angle;    // current rotation angle (used by hazard type)
    bool active;
};

const int MAX_ENEMIES = 4;
Enemy enemies[MAX_ENEMIES];
// ----------------------------------------------------------------------------

int score = 0;
bool gameOver = false;
bool gameStarted = false;
float laneLineOffset = 0.0f;
const float ROAD_SPEED = 0.02f;   // speed the road scrolls down; potholes match this

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

    // Simple trees on grass area (moved out to clear the wider road)
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

void drawCar(float x, float y, bool isPlayer) {
    if (isPlayer) {
        // ── Improved player car (faces up; front = top) ──────────────────

        // Rear spoiler (drawn first so the body overlaps its base)
        glColor3f(0.04f, 0.04f, 0.10f);
        drawRectangle(x, y - 0.15f, 0.22f, 0.03f);
        drawRectangle(x - 0.09f, y - 0.125f, 0.02f, 0.04f);
        drawRectangle(x + 0.09f, y - 0.125f, 0.02f, 0.04f);

        // Tyres
        glColor3f(0.0f, 0.0f, 0.0f);
        drawRectangle(x - 0.095f, y + 0.09f, 0.045f, 0.10f);
        drawRectangle(x + 0.095f, y + 0.09f, 0.045f, 0.10f);
        drawRectangle(x - 0.095f, y - 0.09f, 0.045f, 0.10f);
        drawRectangle(x + 0.095f, y - 0.09f, 0.045f, 0.10f);

        // Hubcaps
        glColor3f(0.72f, 0.72f, 0.78f);
        drawRectangle(x - 0.095f, y + 0.09f, 0.018f, 0.04f);
        drawRectangle(x + 0.095f, y + 0.09f, 0.018f, 0.04f);
        drawRectangle(x - 0.095f, y - 0.09f, 0.018f, 0.04f);
        drawRectangle(x + 0.095f, y - 0.09f, 0.018f, 0.04f);

        // Main body
        glColor3f(0.10f, 0.40f, 1.0f);
        drawRectangle(x, y, 0.17f, 0.30f);

        // Darker side panels for a rounded look
        glColor3f(0.05f, 0.24f, 0.70f);
        drawRectangle(x - 0.077f, y, 0.018f, 0.30f);
        drawRectangle(x + 0.077f, y, 0.018f, 0.30f);

        // Hood highlight (front)
        glColor3f(0.25f, 0.55f, 1.0f);
        drawRectangle(x, y + 0.11f, 0.14f, 0.06f);

        // Cabin / roof
        glColor3f(0.07f, 0.30f, 0.85f);
        drawRectangle(x, y - 0.01f, 0.13f, 0.13f);

        // Windshield (front) and rear window
        glColor3f(0.62f, 0.90f, 1.0f);
        drawRectangle(x, y + 0.055f, 0.11f, 0.05f);
        glColor3f(0.48f, 0.80f, 0.95f);
        drawRectangle(x, y - 0.075f, 0.11f, 0.045f);

        // Twin racing stripes
        glColor3f(1.0f, 1.0f, 1.0f);
        drawRectangle(x - 0.028f, y + 0.115f, 0.014f, 0.05f);
        drawRectangle(x + 0.028f, y + 0.115f, 0.014f, 0.05f);

        // Headlights (front corners)
        glColor3f(1.0f, 1.0f, 0.65f);
        drawRectangle(x - 0.06f, y + 0.14f, 0.03f, 0.02f);
        drawRectangle(x + 0.06f, y + 0.14f, 0.03f, 0.02f);

        // Taillights (rear corners)
        glColor3f(1.0f, 0.12f, 0.12f);
        drawRectangle(x - 0.06f, y - 0.14f, 0.03f, 0.02f);
        drawRectangle(x + 0.06f, y - 0.14f, 0.03f, 0.02f);
    } else {
        glColor3f(1.0f, 0.1f, 0.1f);
        drawRectangle(x, y, 0.18f, 0.28f);

        glColor3f(0.0f, 0.0f, 0.0f);
        drawRectangle(x - 0.07f, y + 0.08f, 0.04f, 0.08f);
        drawRectangle(x + 0.07f, y + 0.08f, 0.04f, 0.08f);
        drawRectangle(x - 0.07f, y - 0.08f, 0.04f, 0.08f);
        drawRectangle(x + 0.07f, y - 0.08f, 0.04f, 0.08f);

        glColor3f(0.6f, 0.9f, 1.0f);
        drawRectangle(x, y + 0.04f, 0.10f, 0.07f);
    }
}

// --- MEMBER 3: Draw enemy car, truck, or pothole hazard ---------------------
void drawEnemy(float x, float y, int type, float angle) {
    if (type == 2) {
        // ── Pothole road hazard (front = bottom is irrelevant; it's on the road) ──
        glPushMatrix();
        glTranslatef(x, y, 0.0f);

        // Cracked asphalt patch around the hole (lighter damaged ring)
        glColor3f(0.22f, 0.22f, 0.22f);
        drawRectangle(0.0f, 0.0f, 0.24f, 0.24f);

        // Swirling dust / debris ring — keeps the rotation transform in use
        glPushMatrix();
        glRotatef(angle, 0.0f, 0.0f, 1.0f);
        glColor3f(0.32f, 0.29f, 0.24f);
        drawRectangle(0.0f, 0.105f, 0.022f, 0.022f);
        drawRectangle(0.0f, -0.105f, 0.022f, 0.022f);
        drawRectangle(0.105f, 0.0f, 0.022f, 0.022f);
        drawRectangle(-0.105f, 0.0f, 0.022f, 0.022f);
        glPopMatrix();

        // Hole edge (dark) with stepped layers for a sense of depth
        glColor3f(0.07f, 0.07f, 0.07f);
        drawRectangle(0.0f, 0.0f, 0.18f, 0.18f);
        glColor3f(0.03f, 0.03f, 0.03f);
        drawRectangle(0.0f, -0.01f, 0.13f, 0.13f);
        glColor3f(0.0f, 0.0f, 0.0f);
        drawRectangle(0.0f, -0.02f, 0.08f, 0.08f);

        // Static cracks radiating out of the rim
        glColor3f(0.28f, 0.28f, 0.28f);
        drawRectangle(0.0f, 0.14f, 0.008f, 0.05f);
        drawRectangle(0.10f, -0.10f, 0.05f, 0.008f);
        drawRectangle(-0.11f, 0.07f, 0.04f, 0.008f);

        glPopMatrix();
    } else if (type == 1) {
        // ── Green delivery truck (faces down: cab at bottom, cargo at top) ──

        // Six wheels (three pairs)
        glColor3f(0.0f, 0.0f, 0.0f);
        drawRectangle(x - 0.10f, y + 0.12f, 0.04f, 0.08f);
        drawRectangle(x + 0.10f, y + 0.12f, 0.04f, 0.08f);
        drawRectangle(x - 0.10f, y + 0.00f, 0.04f, 0.08f);
        drawRectangle(x + 0.10f, y + 0.00f, 0.04f, 0.08f);
        drawRectangle(x - 0.10f, y - 0.12f, 0.04f, 0.08f);
        drawRectangle(x + 0.10f, y - 0.12f, 0.04f, 0.08f);

        // Cargo container (top section)
        glColor3f(0.10f, 0.42f, 0.10f);
        drawRectangle(x, y + 0.07f, 0.20f, 0.22f);

        // Container ribbing
        glColor3f(0.06f, 0.28f, 0.06f);
        drawRectangle(x, y + 0.135f, 0.20f, 0.012f);
        drawRectangle(x, y + 0.070f, 0.20f, 0.012f);
        drawRectangle(x, y + 0.005f, 0.20f, 0.012f);

        // Driver cab (front, brighter green)
        glColor3f(0.16f, 0.56f, 0.16f);
        drawRectangle(x, y - 0.13f, 0.19f, 0.12f);

        // Cab windshield
        glColor3f(0.50f, 0.85f, 0.90f);
        drawRectangle(x, y - 0.155f, 0.14f, 0.05f);

        // Headlights and front bumper
        glColor3f(1.0f, 1.0f, 0.70f);
        drawRectangle(x - 0.07f, y - 0.185f, 0.03f, 0.02f);
        drawRectangle(x + 0.07f, y - 0.185f, 0.03f, 0.02f);
        glColor3f(0.30f, 0.30f, 0.30f);
        drawRectangle(x, y - 0.195f, 0.17f, 0.018f);
    } else {
        // ── Red enemy car (faces down toward the player) ──

        // Tyres
        glColor3f(0.0f, 0.0f, 0.0f);
        drawRectangle(x - 0.085f, y + 0.09f, 0.04f, 0.10f);
        drawRectangle(x + 0.085f, y + 0.09f, 0.04f, 0.10f);
        drawRectangle(x - 0.085f, y - 0.09f, 0.04f, 0.10f);
        drawRectangle(x + 0.085f, y - 0.09f, 0.04f, 0.10f);

        // Hubcaps
        glColor3f(0.62f, 0.62f, 0.66f);
        drawRectangle(x - 0.085f, y + 0.09f, 0.016f, 0.04f);
        drawRectangle(x + 0.085f, y + 0.09f, 0.016f, 0.04f);
        drawRectangle(x - 0.085f, y - 0.09f, 0.016f, 0.04f);
        drawRectangle(x + 0.085f, y - 0.09f, 0.016f, 0.04f);

        // Body
        glColor3f(0.88f, 0.10f, 0.10f);
        drawRectangle(x, y, 0.17f, 0.28f);

        // Darker side panels for depth
        glColor3f(0.58f, 0.05f, 0.05f);
        drawRectangle(x - 0.077f, y, 0.018f, 0.28f);
        drawRectangle(x + 0.077f, y, 0.018f, 0.28f);

        // Hood (front = bottom)
        glColor3f(1.0f, 0.22f, 0.22f);
        drawRectangle(x, y - 0.10f, 0.14f, 0.06f);

        // Cabin / roof
        glColor3f(0.66f, 0.06f, 0.06f);
        drawRectangle(x, y + 0.01f, 0.13f, 0.12f);

        // Windshield (front, lower) and rear window (top)
        glColor3f(0.30f, 0.42f, 0.52f);
        drawRectangle(x, y - 0.05f, 0.11f, 0.05f);
        drawRectangle(x, y + 0.075f, 0.11f, 0.045f);

        // Headlights (front corners, bright)
        glColor3f(1.0f, 1.0f, 0.70f);
        drawRectangle(x - 0.055f, y - 0.135f, 0.03f, 0.02f);
        drawRectangle(x + 0.055f, y - 0.135f, 0.03f, 0.02f);

        // Taillights (rear corners, dark red)
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
    float spawnY = 1.1f + i * 0.65f;   // stagger so they don't all appear at once

    // Lane-overlap fix: pick a lane not already taken by another enemy
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
    enemies[i].type  = rand() % 3;          // 0 = car, 1 = truck, 2 = hazard sign
    enemies[i].angle = 0.0f;
    if (enemies[i].type == 1) {
        enemies[i].width  = 0.20f;
        enemies[i].height = 0.36f;          // trucks are taller/wider
    } else if (enemies[i].type == 2) {
        enemies[i].width  = 0.22f;
        enemies[i].height = 0.22f;          // hazard sign hitbox
    } else {
        enemies[i].width  = 0.18f;
        enemies[i].height = 0.28f;
    }
    if (enemies[i].type == 2) {
        enemies[i].speed = ROAD_SPEED;   // pothole stays locked to the road
    } else {
        enemies[i].speed = 0.015f + (rand() % 5) * 0.002f;  // cars vary slightly
    }
    enemies[i].active = true;
}

// --- MEMBER 3: Initialize all enemies ---------------------------------------
void initEnemies() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        resetEnemy(i);
    }
}
// ----------------------------------------------------------------------------

// --- MEMBER 4: Collision detection (uses enemy array from Member 3) ---------
bool checkCollision() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active &&
            playerX - playerWidth / 2  < enemies[i].x + enemies[i].width / 2 &&
            playerX + playerWidth / 2  > enemies[i].x - enemies[i].width / 2 &&
            playerY - playerHeight / 2 < enemies[i].y + enemies[i].height / 2 &&
            playerY + playerHeight / 2 > enemies[i].y - enemies[i].height / 2) {
            return true;
        }
    }
    return false;
}
// ----------------------------------------------------------------------------

void updateGame(int value) {
    if (gameStarted && !gameOver) {
        laneLineOffset -= ROAD_SPEED;
        if (laneLineOffset < -0.4f) {
            laneLineOffset = 0.0f;
        }

        // --- MEMBER 3: Move all enemies ------------------------------------
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                float newY = enemies[i].y - enemies[i].speed;

                // Anti-overlap: never move into another enemy ahead in the same lane
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

                // Slowly swirl the pothole's debris (rotation transform)
                if (enemies[i].type == 2) {
                    enemies[i].angle += 1.5f;
                    if (enemies[i].angle >= 360.0f) enemies[i].angle -= 360.0f;
                }

                if (enemies[i].y < -1.1f) {
                    score++;
                    resetEnemy(i);
                    for (int j = 0; j < MAX_ENEMIES; j++) {
                        if (enemies[j].type != 2) enemies[j].speed += 0.0005f;
                    }
                }
            }
        }
        // -------------------------------------------------------------------

        if (checkCollision()) {
            gameOver = true;
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
    drawCar(playerX, playerY, true);

    // --- MEMBER 3: Draw all enemies ----------------------------------------
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            drawEnemy(enemies[i].x, enemies[i].y, enemies[i].type, enemies[i].angle);
        }
    }
    // -----------------------------------------------------------------------

    glColor3f(1.0f, 1.0f, 1.0f);

    std::stringstream ss;
    ss << "Score: " << score;
    drawText(-0.95f, 0.9f, ss.str());

    if (gameOver) {
        glColor3f(1.0f, 0.0f, 0.0f);
        drawText(-0.25f, 0.1f, "GAME OVER");

        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(-0.35f, -0.05f, "Press R to Restart");
    }

    glFlush();
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 13) {
        gameStarted = true;
        glutPostRedisplay();
        return;
    }

    if (key == 'a' || key == 'A') {
        playerX -= 0.08f;
    }

    if (key == 'd' || key == 'D') {
        playerX += 0.08f;
    }

    if (key == 'r' || key == 'R') {
        gameOver = false;
        score = 0;
        playerX = 0.0f;
        initEnemies();  // --- MEMBER 3: reset all enemies on restart ---
    }

    if (key == 27) {
        exit(0);
    }

    if (playerX < -0.36f) playerX = -0.36f;
    if (playerX >  0.36f) playerX =  0.36f;

    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT) {
        playerX -= 0.08f;
    }

    if (key == GLUT_KEY_RIGHT) {
        playerX += 0.08f;
    }

    if (playerX < -0.36f) playerX = -0.36f;
    if (playerX >  0.36f) playerX =  0.36f;

    glutPostRedisplay();
}

void init() {
    glClearColor(0.0f, 0.45f, 0.0f, 1.0f);
    srand(time(0));
    initEnemies();  // --- MEMBER 3: initialize all enemies ---
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 700);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Car Dodge Game");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(16, updateGame, 0);

    glutMainLoop();

    return 0;
}
