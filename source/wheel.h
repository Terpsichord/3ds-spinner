#define MAX_OPTIONS 50
#define MAX_OPTION_LEN 256

#define DECELERATION 0.05f


// constants for layout of option list:
#define HEIGHT 35
#define PAD 1.0f
#define BORDER 2.0f
#define CROSS_PAD 4.0f
#define TEXT_HPAD 4.0f
#define TEXT_VPAD 2.0f

typedef struct {
    float centerX;
    float centerY;
    float radius;

    float angle;
    float angularVelocity;

    bool spinning;
    bool finishedSpin;

    char options[MAX_OPTIONS][MAX_OPTION_LEN];
    C2D_Text optionsText[MAX_OPTIONS];

    int selectedOption;
    int numOptions;
} Wheel;

void initWheel(Wheel *w);
void updateWheel(Wheel *w);
void drawWheel(const Wheel *w);
void spinWheelTo(Wheel *w, float angle);

void drawWheelOptions(const Wheel *w, float scrollOffset);
void addWheelOption(Wheel *w);
void modifyWheelOption(Wheel *w, int idx, const char *str);
void removeWheelOption(Wheel *w, int idx);

int getColorIndex(int i, int numSectors, int sectorsPerOption);

void fetchWheelOptions(Wheel *w);
void saveWheelOptions(const Wheel *w);