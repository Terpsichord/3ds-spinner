#define DEG2RAD(angleDegrees) ((angleDegrees) * M_PI / 180.0)
#define ABS(x) ((x) < 0 ? -(x) : (x))

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define NUM_COLORS 6
extern u32 optionColors[NUM_COLORS];
extern C2D_TextBuf dynamicTextBuf;
extern u32 white, gray, black;

#define BOTTOM_WIDTH 320.0f
#define BOTTOM_HEIGHT 240.0f