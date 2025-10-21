#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <algorithm>

#define PI 3.14159265358979323846

// Initial positions and states
float carPosX = 0.0f;     // Initial position of the car on the X-axis
float carSpeed = 6.0f;    // Car speed control for realism (Keys: + / -)
float boatPosX = 0.0f;    // Initial position of the large boat on the X-axis
float miniBoatPosX = -300.0f; // NEW: Initial position of the mini sailboat
float birdPosX = 0.0f;    // Bird starting X position
float birdBasePosY = 300.0f; // Base Y position for bird's flight
bool isOrtho1 = true;     // For toggling between orthographic views (Key: O)

// --- NEW GLOBAL STATE ---
bool isNightMode = false;
bool isBraking = false; // NEW: Flag for brake lights/slowing down

// Array for tree positions (Right side of the road)
float treePositions[][2] = {
    {750.0f, 200.0f},
    {700.0f, 200.0f},
    {650.0f, 200.0f},
};
const int NUM_TREES = sizeof(treePositions) / sizeof(treePositions[0]);

// Array for building positions (Left side of the road, facing the sea)
struct Building {
    float x, y, width, height;
};

Building buildings[] = {
    {100.0f, 200.0f, 60.0f, 80.0f},  // Small building
    {300.0f, 200.0f, 80.0f, 120.0f}, // Tall building
    {450.0f, 200.0f, 50.0f, 70.0f}    // Medium building
};
const int NUM_BUILDINGS = sizeof(buildings) / sizeof(buildings[0]);

// Forward declarations for functions that were missing
void drawSun(float x, float y, float radius);
void drawCloud(float x, float y);
void drawTree(float x, float y);
void drawBirds(float currentBirdY);
void drawStreetLight(float x, float y); // New declaration for street light
void drawBench(float x, float y); // Bench declaration
void drawMiniSailboat(); // NEW: Mini sailboat declaration
void setDayMode();
void setNightMode();
void handleKeyRelease(unsigned char key, int x, int y); // Key release handler

// ---------- MODE SWITCHING FUNCTIONS ----------

void setNightMode() {
    isNightMode = true;
    glClearColor(0.05f, 0.05f, 0.2f, 1.0f); // Dark blue sky
    glutPostRedisplay();
}

void setDayMode() {
    isNightMode = false;
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);  // Light blue sky
    glutPostRedisplay();
}

// ---------- Existing functions ----------

void init() {
    setDayMode(); // Initialize to Day Mode
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (isOrtho1) {
        gluOrtho2D(0, 800, 0, 600);
    } else {
        gluOrtho2D(-100, 900, -100, 700);
    }
}

// Function to toggle ortho projection
void toggleOrtho() {
    isOrtho1 = !isOrtho1;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (isOrtho1) {
        gluOrtho2D(0, 800, 0, 600);
    } else {
        gluOrtho2D(-100, 900, -100, 700);
    }
    glutPostRedisplay();
}

// Function to draw the sea with a gradient for depth
void drawSea() {
    // Sea color changes based on time of day
    if (isNightMode) {
        // Darker sea
        glBegin(GL_POLYGON);
        glColor3f(0.0f, 0.1f, 0.3f);
        glVertex2i(0, 0);
        glVertex2i(800, 0);
        glColor3f(0.0f, 0.15f, 0.4f);
        glVertex2i(800, 150);
        glVertex2i(0, 150);
        glEnd();

    } else {
        // Day sea
        glBegin(GL_POLYGON);
        glColor3f(0.0f, 0.4f, 0.8f);
        glVertex2i(0, 0);
        glVertex2i(800, 0);
        glColor3f(0.0f, 0.5f, 1.0f); // Lighter blue near the road (horizon)
        glVertex2i(800, 150);
        glVertex2i(0, 150);
        glEnd();
    }

    // Simple white wave lines for movement realism
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int i = 0; i < 800; i += 50) {
        glVertex2f(i, 50 + sin((i + boatPosX) * 0.1) * 5);
        glVertex2f(i + 30, 50 + sin((i + 30 + boatPosX) * 0.1) * 5);
    }
    glEnd();
}

// Function to draw the road
void drawRoad() {
    glColor3f(0.3f, 0.3f, 0.3f);  // Gray road
    glBegin(GL_POLYGON);
    glVertex2i(0, 150);
    glVertex2i(800, 150);
    glVertex2i(800, 200);
    glVertex2i(0, 200);
    glEnd();

    // White dashed lines on the road (always white)
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < 800; i += 80) {
        glBegin(GL_LINES);
        glVertex2i(i, 175);
        glVertex2i(i + 40, 175); // Shorter dash
        glEnd();
    }
}

// 🚢 DRAW REALISTIC BOAT 🚢
void drawShip() {
    float waveOffset = sin(boatPosX * 0.015f) * 5.0f;
    float shipYPosition = 65.0f; // Position above water

    glPushMatrix();
    // 1. Translate the entire ship to its position, including the wave oscillation
    glTranslatef(boatPosX, shipYPosition + waveOffset, 0);

    // 2. Set the global scale for the ship
    glScalef(0.7f, 0.7f, 1.0f);

    // --- Hull (Bottom - Submerged Reflection) ---
    glPushMatrix();

    // Apply local flip for the reflection
    glScalef(1.0f, -1.0f, 1.0f);
    // Translate down to position the reflection below the waterline (Y=0)
    glTranslatef(0.0f, -25.0f, 0.0f);

    glColor3f(0.2f, 0.1f, 0.0f); // Darker Brown/Submerged color
    glBegin(GL_POLYGON);
    glVertex2f(-100, 0);
    glVertex2f(-90, 15);
    glVertex2f(-60, 25);
    glVertex2f(80, 25);
    glVertex2f(100, 15);
    glVertex2f(100, 0);
    glEnd();

    glPopMatrix(); // Restore transformation state before the flip

    // --- Hull (Top Part Above Water) ---
    // This is the actual visible hull.
    glColor3f(0.4f, 0.2f, 0.0f); // Brown
    glBegin(GL_POLYGON);
    glVertex2f(-100, 0);
    glVertex2f(-90, 15);
    glVertex2f(-60, 25);
    glVertex2f(80, 25);
    glVertex2f(100, 15);
    glVertex2f(100, 0);
    glEnd();


    // --- Upper Deck (Grey) ---
    // Starts at Y=25
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_POLYGON);
    glVertex2f(-60, 25);
    glVertex2f(80, 25);
    glVertex2f(60, 45);
    glVertex2f(-40, 45);
    glEnd();

    // --- Bridge (White) ---
    // Starts at Y=45
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);
    glVertex2f(-25, 45);
    glVertex2f(45, 45);
    glVertex2f(45, 70);
    glVertex2f(-25, 70);
    glEnd();

    // --- Windows (Light Yellow/Blue) ---
    // Windows are lit up at night
    if (isNightMode) {
        glColor3f(1.0f, 1.0f, 0.8f); // Lit up yellow
    } else {
        glColor3f(0.0f, 0.4f, 0.8f); // Dark blue glass
    }
    for (int i = -20; i <= 40; i += 15) {
        glBegin(GL_POLYGON);
        glVertex2f(i, 55);
        glVertex2f(i + 10, 55);
        glVertex2f(i + 10, 65);
        glVertex2f(i, 65);
        glEnd();
    }

    // --- Chimney (Red) ---
    // Starts at Y=70
    glColor3f(0.8f, 0.1f, 0.1f);
    glBegin(GL_POLYGON);
    glVertex2f(20, 70);
    glVertex2f(30, 70);
    glVertex2f(30, 95);
    glVertex2f(20, 95);
    glEnd();

    // --- Smoke (Light Grey, moving effect) ---
    float smokeY = 95 + sin(glutGet(GLUT_ELAPSED_TIME) / 500.0f) * 5.0f;
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(25, smokeY);
    for (int i = 0; i <= 20; ++i) {
        float ang = i * 2.0f * PI / 20.0f;
        glVertex2f(25 + cos(ang) * 10, smokeY + sin(ang) * 10);
    }
    glEnd();

    glPopMatrix();
}


// ⛵ DRAW MINI SAILBOAT ⛵ (NEW FUNCTION)
void drawMiniSailboat() {
    float waveOffset = sin(miniBoatPosX * 0.05f) * 3.0f;
    float boatYPosition = 120.0f; // Position higher up on the sea for a distant effect

    glPushMatrix();
    glTranslatef(miniBoatPosX, boatYPosition + waveOffset, 0);
    glScalef(0.6f, 0.6f, 1.0f); // EDITED: Increased scale from 0.3f to 0.6f

    // --- Hull (Darker Brown) ---
    glColor3f(0.2f, 0.1f, 0.0f);
    glBegin(GL_POLYGON);
    glVertex2f(-20, 0);
    glVertex2f(20, 0);
    glVertex2f(15, 10);
    glVertex2f(-15, 10);
    glEnd();

    // --- Mast (Thin Black Line) ---
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(0, 10);
    glVertex2f(0, 60);
    glEnd();

    // --- Sail (White/Light) ---
    if (isNightMode) {
        glColor3f(0.6f, 0.6f, 0.7f); // Dim sail at night
    } else {
        glColor3f(1.0f, 1.0f, 1.0f); // Bright white sail
    }
    glBegin(GL_TRIANGLES);
    glVertex2f(0, 60); // Top of mast
    glVertex2f(0, 10); // Base of mast
    glVertex2f(40, 20); // Tip of sail
    glEnd();

    glPopMatrix();
}


// 🚗 DRAW REALISTIC CAR 🚗
void drawRealisticCar() {
    glPushMatrix();
    glTranslatef(carPosX, 0, 0);
    glScalef(1.0f, 1.0f, 1.0f); // Original scale for road visibility

    // Car Body (Red) - Added simple curves
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_POLYGON);
    glVertex2i(45, 200);  // Back bottom
    glVertex2i(125, 200); // Front bottom
    glVertex2i(135, 215); // Hood tip
    glVertex2i(125, 230); // Windshield base front
    glVertex2i(55, 230);  // Windshield base back
    glVertex2i(45, 215);  // Back window base
    glEnd();

    // --- CAR HEADLIGHTS (NEW: Visible ONLY at night) ---
    if (isNightMode) {
        glColor4f(1.0f, 1.0f, 0.8f, 0.8f); // Bright yellow/white
        glBegin(GL_TRIANGLES);
        // Left Headlight Beam
        glVertex2f(135, 208); // Source point (car front)
        glVertex2f(180, 215); // Far wide point
        glVertex2f(180, 195); // Far narrow point

        // Right Headlight Beam (slightly lower source)
        glVertex2f(135, 205); // Source point (car front)
        glVertex2f(180, 212); // Far wide point
        glVertex2f(180, 192); // Far narrow point
        glEnd();

        // Draw the visible light sources on the car
        glColor3f(1.0f, 0.9f, 0.5f);
        glBegin(GL_QUADS);
        glVertex2f(135, 206);
        glVertex2f(137, 206);
        glVertex2f(137, 214);
        glVertex2f(135, 214);
        glEnd();
    }

    // --- CAR BRAKE LIGHTS (NEW: Visible when braking) ---
    if (isBraking) {
        glColor3f(1.0f, 0.0f, 0.0f); // Bright Red
        glBegin(GL_QUADS);
        // Left Brake Light (at x=45)
        glVertex2f(45, 205);
        glVertex2f(40, 205);
        glVertex2f(40, 212);
        glVertex2f(45, 212);
        glEnd();
    }

    // Cabin/Roof (Blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_POLYGON);
    glVertex2i(60, 230);
    glVertex2i(120, 230);
    glVertex2i(110, 245);
    glVertex2i(70, 245);
    glEnd();

    // Windshield (Light Blue/Grey)
    glColor3f(0.7f, 0.8f, 1.0f);
    glBegin(GL_POLYGON);
    glVertex2i(120, 230);
    glVertex2i(110, 245);
    glVertex2i(70, 245);
    glVertex2i(60, 230);
    glEnd();

    // Wheels (Black)
    glColor3f(0.0f, 0.0f, 0.0f);
    // Front wheel
    glBegin(GL_POLYGON);
    for (float angle = 0; angle < 2 * PI; angle += 0.1f) {
        glVertex2f(115 + cos(angle) * 8, 195 + sin(angle) * 8);
    }
    glEnd();
    // Back wheel
    glBegin(GL_POLYGON);
    for (float angle = 0; angle < 2 * PI; angle += 0.1f) {
        glVertex2f(60 + cos(angle) * 8, 195 + sin(angle) * 8);
    }
    glEnd();

    glPopMatrix();
}

// 🏙️ DRAW BUILDING 🏙️
void drawBuilding(float x, float y, float width, float height) {
    glPushMatrix();
    glTranslatef(x, y, 0);

    // Main Body (Light Brown/Tan)
    if (isNightMode) {
        glColor3ub(100, 80, 50); // Darker building at night
    } else {
        glColor3ub(200, 180, 140); // Light Brown/Tan
    }
    glBegin(GL_POLYGON);
    glVertex2f(0, 0);
    glVertex2f(width, 0);
    glVertex2f(width, height);
    glVertex2f(0, height);
    glEnd();

    // Windows (Lit up or Dark)
    if (isNightMode) {
        glColor3f(1.0f, 0.9f, 0.7f); // Yellowish light
    } else {
        glColor3f(0.0f, 0.0f, 0.3f); // Dark blue glass
    }

    float windowW = width / 5.0f;
    float windowH = height / 7.0f;

    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 2; ++c) {
            float winX = windowW + c * (width - 3 * windowW);
            float winY = windowH + r * (height / 3.0f);

            glBegin(GL_POLYGON);
            glVertex2f(winX, winY);
            glVertex2f(winX + windowW, winY);
            glVertex2f(winX + windowW, winY + windowH);
            glVertex2f(winX, winY + windowH);
            glEnd();
        }
    }

    glPopMatrix();
}

// 🕌 DRAW MOSQUE 🕌
void drawMosque(float x, float y) {
    glPushMatrix();
    glTranslatef(x, y, 0);

    // Main Hall (White/Light Grey)
    if (isNightMode) {
        glColor3ub(150, 150, 150); // Slightly dimmed
    } else {
        glColor3ub(230, 230, 230);
    }
    glBegin(GL_POLYGON);
    glVertex2f(0, 0);
    glVertex2f(60, 0);
    glVertex2f(60, 40);
    glVertex2f(0, 40);
    glEnd();

    // Dome (Green)
    glColor3f(0.0f, 0.4f, 0.0f);
    int numSegments = 30;
    float radius = 20.0f;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(30, 40); // Center of the base (where the dome meets the hall)
    for (int i = 0; i <= numSegments; i++) {
        float angle = i * PI / numSegments;
        glVertex2f(30 + cos(angle) * radius, 40 + sin(angle) * radius);
    }
    glEnd();

    // Minaret (Tall Tower)
    glColor3ub(180, 180, 180);
    glBegin(GL_POLYGON);
    glVertex2f(60, 0);
    glVertex2f(70, 0);
    glVertex2f(70, 100);
    glVertex2f(60, 100);
    glEnd();

    // Minaret top (cone)
    glColor3f(0.0f, 0.4f, 0.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(65, 120);
    glVertex2f(60, 100);
    glVertex2f(70, 100);
    glEnd();

    glPopMatrix();
}

// 🎠 DRAW PLAYGROUND 🎠
void drawPlayground(float x, float y) {
    glPushMatrix();
    glTranslatef(x, y, 0);

    // Ground (Sand color)
    if (isNightMode) {
        glColor3ub(120, 100, 60); // Dark sand
    } else {
        glColor3ub(240, 220, 160);
    }
    glBegin(GL_POLYGON);
    glVertex2f(-50, 0);
    glVertex2f(100, 0);
    glVertex2f(100, 20);
    glVertex2f(-50, 20);
    glEnd();

    // --- FENCE BOUNDARY (NEW) ---
    glColor3ub(100, 100, 100); // Gray fence color
    glLineWidth(2.0f);
    float fenceHeight = 35.0f;
    float postSpacing = 15.0f;

    // Vertical Posts
    glBegin(GL_LINES);
    for (float i = -50; i <= 100; i += postSpacing) {
        glVertex2f(i, 20);
        glVertex2f(i, 20 + fenceHeight);
    }
    glEnd();

    // Horizontal top rail
    glBegin(GL_LINES);
    glVertex2f(-50, 20 + fenceHeight);
    glVertex2f(100, 20 + fenceHeight);
    glEnd();

    // Horizontal middle rail
    glBegin(GL_LINES);
    glVertex2f(-50, 20 + fenceHeight/2.0f);
    glVertex2f(100, 20 + fenceHeight/2.0f);
    glEnd();


    // --- Swing Set ---
    // Posts (Grey)
    glColor3f(0.5f, 0.5f, 0.5f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(0, 20);
    glVertex2f(0, 60);
    glVertex2f(50, 20);
    glVertex2f(50, 60);
    glEnd();

    // Top Bar
    glBegin(GL_LINES);
    glVertex2f(0, 60);
    glVertex2f(50, 60);
    glEnd();

    // Swing Ropes (Black)
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glVertex2f(15, 60);
    glVertex2f(15, 40);
    glVertex2f(35, 60);
    glVertex2f(35, 40);
    glEnd();

    // Swing Seat (Yellow)
    glColor3ub(255, 200, 0);
    glBegin(GL_POLYGON);
    glVertex2f(10, 40);
    glVertex2f(40, 40);
    glVertex2f(40, 35);
    glVertex2f(10, 35);
    glEnd();

    // --- Slide ---
    // Stairs (Brown)
    glColor3f(0.6f, 0.3f, 0.0f);
    glBegin(GL_POLYGON);
    glVertex2f(80, 20);
    glVertex2f(85, 20);
    glVertex2f(85, 50);
    glVertex2f(80, 50);
    glEnd();

    // Slide chute (Blue)
    glColor3f(0.0f, 0.5f, 0.8f);
    glBegin(GL_POLYGON);
    glVertex2f(85, 50);
    glVertex2f(70, 30);
    glVertex2f(75, 30);
    glVertex2f(85, 55);
    glEnd();

    glPopMatrix();
}

// 🌳 DRAW TREE 🌳 (Restored Definition)
void drawTree(float x, float y) {
    // Draw the trunk
    glColor3f(0.55f, 0.27f, 0.07f);  // Brown color for the trunk
    glBegin(GL_POLYGON);
    glVertex2f(x - 10, y);
    glVertex2f(x + 10, y);
    glVertex2f(x + 10, y + 40);
    glVertex2f(x - 10, y + 40);
    glEnd();

    // Draw the foliage (overlapping circles/fans for a bushier look)
    if (isNightMode) {
        glColor3f(0.0f, 0.2f, 0.0f); // Dark green
    } else {
        glColor3f(0.0f, 0.5f, 0.0f);  // Bright green
    }

    int numSegments = 20;
    float radius = 30.0f;

    // Bottom-left part
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x - 15, y + 40 + 10);
    for (int i = 0; i <= numSegments; i++) {
        float angle = i * 2.0f * PI / numSegments;
        glVertex2f(x - 15 + cos(angle) * radius, y + 40 + 10 + sin(angle) * radius);
    }
    glEnd();

    // Top-center part
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y + 40 + 30);
    for (int i = 0; i <= numSegments; i++) {
        float angle = i * 2.0f * PI / numSegments;
        glVertex2f(x + cos(angle) * radius * 1.2, y + 40 + 30 + sin(angle) * radius * 1.2);
    }
    glEnd();

    // Bottom-right part
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x + 15, y + 40 + 10);
    for (int i = 0; i <= numSegments; i++) {
        float angle = i * 2.0f * PI / numSegments;
        glVertex2f(x + 15 + cos(angle) * radius, y + 40 + 10 + sin(angle) * radius);
    }
    glEnd();
}

// 🐦 DRAW BIRDS 🐦 (Restored Definition)
void drawBirds(float currentBirdY) {
    if (isNightMode) return; // Hide birds at night

    glPushMatrix();
    // The bird's Y position now oscillates around birdBasePosY
    glTranslatef(birdPosX, currentBirdY, 0);

    glColor3f(0.0f, 0.0f, 0.0f);  // Black color for birds
    glLineWidth(2.0f); // Thicker lines for better visibility
    // Draw birds as simple "V" shapes
    glBegin(GL_LINES);
    // First bird
    glVertex2f(0.0f, 0.0f);
    glVertex2f(10.0f, 10.0f);
    glVertex2f(10.0f, 10.0f);
    glVertex2f(20.0f, 0.0f);

    // Second bird
    glVertex2f(30.0f, 5.0f);
    glVertex2f(40.0f, 15.0f);
    glVertex2f(40.0f, 15.0f);
    glVertex2f(50.0f, 5.0f);
    glEnd();

    glPopMatrix();
}

// DRAW BENCH FUNCTION (NEW)
void drawBench(float x, float y) {
    glPushMatrix();
    glTranslatef(x, y, 0);

    // Seat (Brown wood)
    glColor3ub(139, 69, 19);
    glBegin(GL_QUADS);
    glVertex2f(-30, 0);
    glVertex2f(30, 0);
    glVertex2f(30, 5);
    glVertex2f(-30, 5);
    glEnd();

    // Legs (Black/Dark metal)
    glColor3f(0.2f, 0.2f, 0.2f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    // Back left
    glVertex2f(-25, 0);
    glVertex2f(-25, -15);
    // Back right
    glVertex2f(25, 0);
    glVertex2f(25, -15);
    glEnd();

    glPopMatrix();
}


// Timer function to update positions
void update(int value) {
    // Update car position with controlled speed
    carPosX += carSpeed;
    if (carPosX > 850) {
        carPosX = -120;  // Reset car position
    }

    // Update large boat position
    boatPosX += 2.0f;
    if (boatPosX > 800) {
        boatPosX = -600;  // Reset boat position
    }

    // NEW: Update mini boat position (slower movement)
    miniBoatPosX += 0.8f;
    if (miniBoatPosX > 850) {
        miniBoatPosX = -100; // Reset mini boat position
    }


    // Update bird position (oscillating flight path)
    birdPosX += 3.0f;
    float time_factor = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    birdBasePosY = 300.0f + sin(time_factor * 2.0f) * 50.0f;

    if (birdPosX > 850) {
        birdPosX = -50;  // Reset bird position once off-screen
    }

    glutPostRedisplay();  // Redraw the scene
    glutTimerFunc(30, update, 0);  // Call update again after 30 ms
}

// Keyboard key-down function
void handleKeypress(unsigned char key, int x, int y) {
    if (key == 'o' || key == 'O') {
        toggleOrtho();
    } else if (key == 's' || key == 'S') {
        #ifdef _WIN32
             // Beep(750, 300);
        #else
             std::cout << "\a" << std::flush;
        #endif
    } else if (key == '+') {
        carSpeed = std::min(carSpeed + 1.0f, 15.0f);
        std::cout << "Car Speed: " << carSpeed << std::endl;
    } else if (key == '-') {
        carSpeed = std::max(carSpeed - 1.0f, 1.0f);
        std::cout << "Car Speed: " << carSpeed << std::endl;
    } else if (key == 'n' || key == 'N') { // Toggle Night Mode
        if (isNightMode) {
            setDayMode();
        } else {
            setNightMode();
        }
    } else if (key == 'b' || key == 'B') { // NEW: Brake Activation
        isBraking = true;
        carSpeed = std::max(carSpeed - 3.0f, 1.0f); // Slow down significantly
        std::cout << "Car Braking. Speed: " << carSpeed << std::endl;
    }
}

// NEW: Keyboard key-up function for releasing the brake
void handleKeyRelease(unsigned char key, int x, int y) {
    if (key == 'b' || key == 'B') {
        isBraking = false;
        // Restore speed slightly, capped at 6.0f (default cruise speed)
        carSpeed = std::min(carSpeed + 2.0f, 6.0f);
        std::cout << "Brakes released. Speed: " << carSpeed << std::endl;
    }
}

// Mouse function for interaction (unchanged)
void handleMouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // ...
    }
}

// DRAW STREET LIGHT FUNCTION
void drawStreetLight(float x, float y) {
    glPushMatrix();
    glTranslatef(x, y, 0);

    // Pole (Grey)
    glColor3f(0.4f, 0.4f, 0.4f);
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    glVertex2f(0, 0);
    glVertex2f(0, 100); // Height
    glEnd();

    // Arm (Grey)
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(0, 100);
    glVertex2f(20, 100);
    glEnd();

    // Lamp Head (Light color depends on mode)
    if (isNightMode) {
        glColor3f(1.0f, 0.9f, 0.5f); // Bright warm light
    } else {
        glColor3f(0.4f, 0.4f, 0.4f); // Dim grey/off during day
    }

    glBegin(GL_POLYGON);
    glVertex2f(20, 100);
    glVertex2f(25, 95);
    glVertex2f(25, 105);
    glEnd();

    glPopMatrix();
}

// Display callback
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw all background elements first
    drawSea();
    drawRoad();

    if (!isNightMode) {
        drawSun(700.0f, 500.0f, 40.0f); // Sun only visible during day
    } else {
        // Draw Moon at night
        const int segments = 40;
        glPushMatrix();
        glTranslatef(700.0f, 500.0f, 0);
        glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.8f, 0.8f, 0.8f); // White/Grey moon
        glVertex2f(0.0f, 0.0f);
        for (int i = 0; i <= segments; ++i) {
            float ang = (float)i / (float)segments * 2.0f * PI;
            glVertex2f(cos(ang) * 40.0f, sin(ang) * 40.0f);
        }
        glEnd();
        glPopMatrix();
    }

    drawCloud(150.0f, 500.0f);
    drawCloud(400.0f, 550.0f);
    drawCloud(600.0f, 480.0f);

    // --- DRAW STATIC STRUCTURES (Non-moving objects) ---

    // Draw Buildings on the left side of the road
    for (int i = 0; i < NUM_BUILDINGS; ++i) {
        drawBuilding(buildings[i].x, buildings[i].y, buildings[i].width, buildings[i].height);
    }

    // Draw Street Lights along the left side of the road (Base Y=200)
    drawStreetLight(150.0f, 200.0f);
    drawStreetLight(350.0f, 200.0f);
    drawStreetLight(550.0f, 200.0f);

    // Draw Mosque
    drawMosque(20.0f, 200.0f);

    // Draw Playground
    drawPlayground(500.0f, 200.0f);

    // NEW: Draw Bench near the trees/playground
    drawBench(620.0f, 200.0f);

    // Draw Trees on the right side of the road
    for (int i = 0; i < NUM_TREES; ++i) {
        drawTree(treePositions[i][0], treePositions[i][1]); // Now defined
    }

    // Draw moving objects last to ensure they are on top
    drawMiniSailboat(); // NEW: Draw the smaller sailboat first (appears farther away)
    drawShip(); // Draw the main ship second
    drawRealisticCar();
    drawBirds(birdBasePosY); // Now defined

    glutSwapBuffers();
}

// ----------------- NEW/ADDED: drawSun and drawCloud -----------------

void drawSun(float x, float y, float radius) {
    const int segments = 40;
    glPushMatrix();
    glTranslatef(x, y, 0);
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(1.0f, 0.9f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    for (int i = 0; i <= segments; ++i) {
        float ang = (float)i / (float)segments * 2.0f * PI;
        glVertex2f(cos(ang) * radius, sin(ang) * radius);
    }
    glEnd();
    glPopMatrix();
}

void drawCloud(float x, float y) {
    // Cloud color changes slightly at night
    float r = 1.0f, g = 1.0f, b = 1.0f;
    if (isNightMode) {
        r = 0.6f; g = 0.6f; b = 0.7f;
    }

    // Simple cloud built from overlapping circle fans
    int segments = 20;
    float radii[] = {30.0f, 28.0f, 24.0f};
    float offsets[] = { -30.0f, 0.0f, 30.0f };

    glPushMatrix();
    glTranslatef(x, y, 0);
    glColor3f(r, g, b);
    for (int c = 0; c < 3; ++c) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(offsets[c], 0.0f);
        for (int i = 0; i <= segments; ++i) {
            float ang = (float)i / (float)segments * 2.0f * PI;
            glVertex2f(offsets[c] + cos(ang) * radii[c], sin(ang) * radii[c] * 0.6f);
        }
        glEnd();
    }
    glPopMatrix();
}

// Main function (updated to register handleKeyRelease)
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(850, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Realistic Scene Animation");

    init();

    glutDisplayFunc(display);
    glutTimerFunc(30, update, 0);
    glutKeyboardFunc(handleKeypress);
    glutKeyboardUpFunc(handleKeyRelease); // REGISTERED NEW KEY-UP HANDLER
    glutMouseFunc(handleMouse);

    glutMainLoop();
    return 0;
}
