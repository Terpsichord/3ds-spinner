#include <3ds.h>
#include <citro2d.h>

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "wheel.h"
#include "main.h"

static void updateWheelOptions(Wheel *w) {
    C2D_TextBufClear(dynamicTextBuf);
    for (int i = 0; i < w->numOptions; i++) {
        C2D_TextParse(&w->optionsText[i], dynamicTextBuf, w->options[i]);
        C2D_TextOptimize(&w->optionsText[i]);
    }
}


void initWheel(Wheel *w) {
    w->centerX = 200.0f;
    w->centerY = 120.0f;
    w->radius = 100.0f;

    w->angle = 0.0f;
    w->angularVelocity = 0.0f;

    w->spinning = false;
    w->finishedSpin = false;


    w->selectedOption = 0;
    strncpy(w->options[0], "Option 1", MAX_OPTION_LEN);
    strncpy(w->options[1], "Option 2", MAX_OPTION_LEN);
    strncpy(w->options[2], "Option 3", MAX_OPTION_LEN);
    w->numOptions = 3;

    updateWheelOptions(w);
}



int getColorIndex(int i, int numOptions, int sectorsPerOption, bool duplicated) {
    if (duplicated) {
        i %= numOptions;
    }

    int idx = i % NUM_COLORS / sectorsPerOption;

    // this is to prevent two of the same color next to each other
    if (numOptions % NUM_COLORS == 1 && i == numOptions - 1 && numOptions != 1) {
        idx++;
    }

    return idx;
}

void drawWheel(const Wheel *w) {
    if (w->numOptions == 0) return;

    int sectorsPerOption = w->numOptions == 1 ? 4 : w->numOptions > 3 ? 1 : 2;

    float radius = w->radius;
    if (w->numOptions <= 3) {
        // looks weird going from 4 to 3 colors without this
        radius *= 0.9f;
    }

    int numOptions = MAX(w->numOptions * sectorsPerOption, 1);
    int numSectors = numOptions * (w->duplicated ? 3 : 1);

    float anglePerSector = 360.0f / numSectors;

    for (int i = 0; i < numSectors; i++) {
        float a1 = DEG2RAD(i * anglePerSector + w->angle);
        float a2 = DEG2RAD((i + 1) * anglePerSector + w->angle);

        float x1 = w->centerX + radius * cosf(a1);
        float y1 = w->centerY - radius * sinf(a1);
        float x2 = w->centerX + radius * cosf(a2);
        float y2 = w->centerY - radius * sinf(a2);

        int colorIdx = getColorIndex(i, numOptions, sectorsPerOption, w->duplicated);

        C2D_DrawTriangle(
                w->centerX, w->centerY, optionColors[colorIdx],
                x1, y1, optionColors[colorIdx],
                x2, y2, optionColors[colorIdx],
                0
        );
    }

    C2D_DrawTriangle(w->centerX - 10.5f, w->centerY - 10.5f, black,
                     w->centerX + 10.5f, w->centerY - 10.5f, black,
                     w->centerX, w->centerY - 24.0f, black, 0);
}

void updateWheel(Wheel *w) {
    w->angle += w->angularVelocity;
    if (w->angle >= 360.0f) w->angle -= 360.0f;
    w->angularVelocity -= DECELERATION;
    if (w->angularVelocity <= 0.0f) {
        w->spinning = false;
        w->angularVelocity = 0.0f;

        float sectors = w->numOptions * (w->duplicated ? 3 : 1);
        w->selectedOption = (int) (fmodf(450.0f - w->angle, 360.0f) / (360 / sectors));
        w->selectedOption %= w->numOptions;
        w->finishedSpin = true;
    }
}

void spinWheelTo(Wheel *w, float angle) {
    // the angular velocity needed to get to the angle
    w->angularVelocity = sqrtf(2 * DECELERATION * (angle - w->angle + 1080.0f));
    w->spinning = true;
}

static void drawCross(float x, float y, float size, float t, u32 color) {
    float cx = x + size / 2.0f;
    float cy = y + size / 2.0f;

    float dx = size / 2.0f;
    float dy = size / 2.0f;

    float diag_len = sqrtf(dx * dx + dy * dy);
    float tx = t * dx / diag_len;
    float ty = t * dy / diag_len;

    // Top left to bottom right (\)
    C2D_DrawTriangle(
            cx - dx - tx, cy - dy + ty, color,
            cx + dx - tx, cy + dy + ty, color,
            cx - dx + tx, cy - dy - ty, color,
            0
    );
    C2D_DrawTriangle(
            cx - dx + tx, cy - dy - ty, color,
            cx + dx - tx, cy + dy + ty, color,
            cx + dx + tx, cy + dy - ty, color,
            0
    );

    // Bottom left to top right (/)
    C2D_DrawTriangle(
            cx + dx - tx, cy - dy - ty, color,
            cx - dx - tx, cy + dy - ty, color,
            cx + dx + tx, cy - dy + ty, color,
            0
    );
    C2D_DrawTriangle(
            cx + dx + tx, cy - dy + ty, color,
            cx - dx - tx, cy + dy - ty, color,
            cx - dx + tx, cy + dy + ty, color,
            0
    );
}

void drawWheelOptions(const Wheel *w, float scrollOffset) {
    for (int i = 0; i < w->numOptions; i++) {
        int colorIdx = getColorIndex(i, w->numOptions, 1, false);

        // main border
        C2D_DrawRectSolid(PAD,
                          (HEIGHT + PAD) * i + PAD - scrollOffset, 0.0f,
                          BOTTOM_WIDTH - 2 * PAD - HEIGHT,
                          HEIGHT,
                          optionColors[colorIdx]);
        // main inner
        C2D_DrawRectSolid(PAD + BORDER,
                          (HEIGHT + PAD) * i + PAD + BORDER - scrollOffset, 0.0f,
                          BOTTOM_WIDTH - 2 * PAD - 2 * BORDER - HEIGHT,
                          HEIGHT - 2 * BORDER,
                          white);

        C2D_DrawText(&w->optionsText[i], 0, PAD + BORDER + TEXT_HPAD, (HEIGHT + PAD) * i + PAD + BORDER + TEXT_VPAD - scrollOffset, 0.0f, 0.8f, 0.8f);

        // main cover
        C2D_DrawRectSolid(BOTTOM_WIDTH - PAD - HEIGHT - BORDER - TEXT_HPAD,
                          (HEIGHT + PAD) * i + PAD + BORDER - scrollOffset, 0.0f,
                          TEXT_HPAD,
                          HEIGHT - 2 * BORDER,
                          white);

        // cross border
        C2D_DrawRectSolid(BOTTOM_WIDTH - PAD - BORDER - HEIGHT,
                          (HEIGHT + PAD) * i + PAD - scrollOffset, 0.0f,
                          HEIGHT + BORDER,
                          HEIGHT,
                          optionColors[colorIdx]);

        // cross inner
        C2D_DrawRectSolid(BOTTOM_WIDTH - PAD - HEIGHT,
                          (HEIGHT + PAD) * i + PAD + BORDER - scrollOffset, 0.0f,
                          HEIGHT - BORDER,
                          HEIGHT - 2 * BORDER,
                          white);

        drawCross(BOTTOM_WIDTH - PAD - HEIGHT + CROSS_PAD,
                  (HEIGHT + PAD) * i + PAD + BORDER + CROSS_PAD - scrollOffset,
                  HEIGHT - 2 * BORDER - 2 * CROSS_PAD,
                  1.0f, black);

    }

    // edge cover
    C2D_DrawRectSolid(BOTTOM_WIDTH - PAD,
                      0.0f, 0.0f,
                      PAD,
                      BOTTOM_HEIGHT,
                      white);

}

void addWheelOption(Wheel *w) {
    if (w->numOptions >= MAX_OPTIONS) return;

    w->numOptions++;

    updateWheelOptions(w);
}

void modifyWheelOption(Wheel *w, int idx, const char *str) {
    strncpy(w->options[idx], str, MAX_OPTION_LEN);
    updateWheelOptions(w);
}

void removeWheelOption(Wheel *w, int idx) {
    if (w->numOptions <= 0) return;

    w->numOptions--;

    for (int i = idx; i < w->numOptions; i++) {
        strncpy(w->options[i], w->options[i + 1], MAX_OPTION_LEN);
    }

    updateWheelOptions(w);
}

void shuffleWheelOptions(Wheel *w) {
    for (int i = 0; i < w->numOptions; i++) {
        int j = rand() % (i + 1);
        char tmp[MAX_OPTION_LEN];
        C2D_Text tmpText;

        strncpy(tmp, w->options[i], MAX_OPTION_LEN - 1);
        tmpText = w->optionsText[i];

        strncpy(w->options[i], w->options[j], MAX_OPTION_LEN - 1);
        w->optionsText[i] = w->optionsText[j];

        strncpy(w->options[j], tmp, MAX_OPTION_LEN - 1);
        w->optionsText[j] = tmpText;
    }
}

void fetchWheelOptions(Wheel *w) {
    FILE *f = fopen("sdmc:/3ds/3ds-spinner/options.txt", "r");
    if (!f) return;

    char line[MAX_OPTION_LEN];
    int i = 0;
    while (fgets(line, MAX_OPTION_LEN, f) != NULL) {
        line[strcspn(line, "\n")] = 0;
        strncpy(w->options[i], line, MAX_OPTION_LEN);
        i++;
    }
    w->numOptions = i;
    fclose(f);

    if (w->numOptions == 0) {
        initWheel(w);
    } else {
        updateWheelOptions(w);
    }

    f = fopen("sdmc:/3ds/3ds-spinner/duplicated", "r");
    if (f) {
        w->duplicated = true;
        fclose(f);
    }
}

void makeMissingDir(const char *path) {
    struct stat st;

    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        // already exists
        return;
    }

    mkdir(path, 0777);
}

void saveWheelOptions(const Wheel *w) {
    makeMissingDir("sdmc:/3ds/3ds-spinner");
    FILE *f = fopen("sdmc:/3ds/3ds-spinner/options.txt", "w");
    if (!f) goto next;

    if (w->numOptions == 0) {
        fclose(f);
        goto next;
    }

    for (int i = 0; i < w->numOptions - 1; i++) {
        fprintf(f, "%s\n", w->options[i]);
    }
    fprintf(f, "%s", w->options[w->numOptions - 1]);
    fclose(f);

    next:
    if (w->duplicated) {
        f = fopen("sdmc:/3ds/3ds-spinner/duplicated", "w");
        if (f) fclose(f);
    } else {
        remove("sdmc:/3ds/3ds-spinner/duplicated");
    }
}

