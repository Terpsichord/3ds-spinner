#include <3ds.h>
#include <citro2d.h>
#include <stdlib.h>
#include <time.h>

#include "wheel.h"
#include "main.h"

u32 white, gray, black, darkGray, scrollGray;
u32 optionColors[NUM_COLORS];
bool darkText[NUM_COLORS] = { false, false, false, true /* yellow */,  false, true /* cyan */ };

C2D_TextBuf staticTextBuf, dynamicTextBuf;
C2D_Text selectedText, continueText, removeText, addText, aText, duplicateText, hideText, shuffleText;

void initGfx(C3D_RenderTarget **top, C3D_RenderTarget **bottom) {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    *bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
}

void initColors(void) {
    white = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    gray = C2D_Color32(0xE0, 0xE0, 0xE0, 0xFF);
    darkGray = C2D_Color32(0x40, 0x40, 0x40, 0xFF);
    scrollGray = C2D_Color32(0x40, 0x40, 0x40, 0xE0);
    black = C2D_Color32(0x00, 0x00, 0x00, 0xFF);

    optionColors[0] = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
    optionColors[1] = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);
    optionColors[2] = C2D_Color32(0x00, 0x00, 0xFF, 0xFF);
    optionColors[3] = C2D_Color32(0xFF, 0xFF, 0x00, 0xFF);
    optionColors[4] = C2D_Color32(0xFF, 0x00, 0xFF, 0xFF);
    optionColors[5] = C2D_Color32(0x00, 0xFF, 0xFF, 0xFF);
}

void initText(void) {
    staticTextBuf = C2D_TextBufNew(1024);
    dynamicTextBuf = C2D_TextBufNew(4096);

    C2D_TextParse(&selectedText, staticTextBuf, "selected");
    C2D_TextParse(&continueText, staticTextBuf, "\uE000 Continue");
    C2D_TextParse(&removeText, staticTextBuf, "\uE002 Remove and continue");
    C2D_TextParse(&addText, staticTextBuf, "Press \uE003 to add a new option");
    C2D_TextParse(&aText, staticTextBuf, "\uE000");
    C2D_TextParse(&duplicateText, staticTextBuf, "Duplicate x3");
    C2D_TextParse(&hideText, staticTextBuf, "Hide options");
    C2D_TextParse(&shuffleText, staticTextBuf, "Shuffle");

    C2D_TextOptimize(&selectedText);
    C2D_TextOptimize(&continueText);
    C2D_TextOptimize(&removeText);
    C2D_TextOptimize(&addText);
    C2D_TextOptimize(&aText);
    C2D_TextOptimize(&duplicateText);
    C2D_TextOptimize(&hideText);
    C2D_TextOptimize(&shuffleText);
}

void finish(void) {
    C2D_TextBufDelete(staticTextBuf);
    C2D_TextBufDelete(dynamicTextBuf);

    C2D_Fini();
    C3D_Fini();
    gfxExit();
}

void drawPopup(const C2D_Text *text, u32 color, bool useDarkText) {
    float textWidth, textHeight, textScale = 1.0f;
    C2D_TextGetDimensions(text, textScale, textScale, &textWidth, &textHeight);

    float colorWidth = MAX(textWidth + 20.0f, 200.0f);
    if (colorWidth > 370.0f) {
        textScale = 350.0f / textWidth;
        colorWidth = 370.0f;
    }

    float grayWidth = MAX(colorWidth + 20.0f, 320.0f);

    C2D_DrawRectSolid(200.0f - grayWidth / 2.0f, 50.0f, 0.0f, grayWidth, 140.0f, gray);
    C2D_DrawRectSolid(200.0f - colorWidth / 2.0f, 80.0f, 0.0f, colorWidth, 50.0f, color);

    C2D_DrawText(text, C2D_WithColor, 210.0f - colorWidth / 2.0f, 105.0f - textScale * textHeight / 2, 0.0f, textScale, textScale, useDarkText ? black : white);

    C2D_DrawText(&selectedText, 0, 173.0f, 130.0f, 0.0f, 0.5f, 0.5f);
    C2D_DrawText(&continueText, 0, 80.0f, 170.0f, 0.0f, 0.5f, 0.5f);
    C2D_DrawText(&removeText, 0, 185.0f, 170.0f, 0.0f, 0.5f, 0.5f);
}

void drawScrollBar(float scroll, float maxScroll) {
    float height = (BOTTOM_HEIGHT - BAR_HEIGHT) / (maxScroll + BOTTOM_HEIGHT - BAR_HEIGHT) * (BOTTOM_HEIGHT - 2 * PAD - BAR_HEIGHT);
    float offset = PAD + scroll / maxScroll * (BOTTOM_HEIGHT - 2 * PAD - BAR_HEIGHT - height);

    C2D_DrawRectSolid(BOTTOM_WIDTH - 6.0f, offset, 0.0f, 2.0f, height, scrollGray);
}

void drawBar(bool duplicated, bool hidden, bool shuffled) {
    C2D_DrawRectSolid(0.0f, BOTTOM_HEIGHT - BAR_HEIGHT, 0.0f, BOTTOM_WIDTH, BAR_HEIGHT, white);
    C2D_DrawRectSolid(0.0f, BOTTOM_HEIGHT - BAR_HEIGHT, 0.0f, BOTTOM_WIDTH, 2.0f, gray);

    C2D_DrawRectSolid(BTN_HPAD, BOTTOM_HEIGHT - BAR_HEIGHT + BTN_VPAD, 0.0f, BTN_WIDTH - 2, BAR_HEIGHT - 2 * BTN_VPAD, duplicated ? darkGray : gray);
    C2D_DrawText(&duplicateText, C2D_WithColor | C2D_AlignCenter, BTN_HPAD + BTN_WIDTH / 2, BOTTOM_HEIGHT - BAR_HEIGHT + BTN_VPAD + TEXT_VPAD, 0.0f, 0.5f, 0.5f, duplicated ? white : black);

    C2D_DrawRectSolid(BTN_HPAD + BTN_WIDTH, BOTTOM_HEIGHT - BAR_HEIGHT + BTN_VPAD, 0.0f, BTN_WIDTH - 2, BAR_HEIGHT - 2 * BTN_VPAD, hidden ? darkGray : gray);
    C2D_DrawText(&hideText, C2D_WithColor | C2D_AlignCenter, BTN_HPAD + BTN_WIDTH + BTN_WIDTH / 2, BOTTOM_HEIGHT - BAR_HEIGHT + BTN_VPAD + TEXT_VPAD, 0.0f, 0.5f, 0.5f, hidden ? white : black);

    C2D_DrawRectSolid(BTN_HPAD + 2 * BTN_WIDTH, BOTTOM_HEIGHT - BAR_HEIGHT + BTN_VPAD, 0.0f, BTN_WIDTH - 2, BAR_HEIGHT - 2 * BTN_VPAD, shuffled ? darkGray : gray);
    C2D_DrawText(&shuffleText, C2D_WithColor | C2D_AlignCenter, BTN_HPAD + 2 * BTN_WIDTH + BTN_WIDTH / 2, BOTTOM_HEIGHT - BAR_HEIGHT + BTN_VPAD + TEXT_VPAD, 0.0f, 0.5f, 0.5f, shuffled ? white : black);
}

void render(C3D_RenderTarget *top, C3D_RenderTarget *bottom, const Wheel *wheel, float scroll, float maxScroll, bool hidden, bool shuffleHeld) {
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

    C2D_TargetClear(top, white);
    C2D_TargetClear(bottom, white);

    C2D_SceneBegin(top);
    {
        drawWheel(wheel);
        if (wheel->finishedSpin) {
            drawPopup(&wheel->optionsText[wheel->selectedOption],
                      optionColors[getColorIndex(wheel->selectedOption, wheel->numOptions, 1, false)],
                      darkText[wheel->selectedOption]);
        } else {
            if (wheel->numOptions > 0) {
                C2D_DrawCircleSolid(wheel->centerX, wheel->centerY, 0.0f, 15.0f, black);
            }
            if (!wheel->spinning) {
                C2D_DrawText(&aText, C2D_WithColor, 188.5f, 105.0f, 0.0f, 1.0f, 1.0f, white);
                C2D_DrawText(&addText, 0, 115.0f, 220.0f, 0.0f, 0.5f, 0.5f);
            }
        }
    }

    C2D_SceneBegin(bottom);
    {
        if (!wheel->spinning && !wheel->finishedSpin) {
            if (!hidden) {
                drawWheelOptions(wheel, scroll);
                if (maxScroll > 0.0f) {
                    drawScrollBar(scroll, maxScroll);
                }
            }
            drawBar(wheel->duplicated, hidden, shuffleHeld);
        }
    }

    C3D_FrameEnd(0);
}

void handleTouch(Wheel *wheel, float *scroll, bool *barHeld, bool *hidden, int *shuffleHeld) {
    // code borrowed and modified from
    // https://github.com/devkitPro/3ds-hbmenu/blob/master/source/ui/menu.c#L209

    static int heldTime = 0;
    static touchPosition firstTouch, prevTouch;

    touchPosition touchPos;
    hidTouchRead(&touchPos);

    u32 kDown = hidKeysDown();
    u32 kHeld = hidKeysHeld();
    u32 kUp = hidKeysUp();

    if (kDown & KEY_TOUCH) {
        heldTime = 0;
        firstTouch = touchPos;
        if (touchPos.py >= BOTTOM_HEIGHT - BAR_HEIGHT) {
            *barHeld = true;

            if (touchPos.py >= BOTTOM_HEIGHT - BAR_HEIGHT + BTN_VPAD && touchPos.py < BOTTOM_HEIGHT - BTN_VPAD) {

                if (touchPos.px >= BTN_HPAD && touchPos.px < BTN_HPAD + BTN_WIDTH - 2) {
                    wheel->duplicated = !wheel->duplicated;
                } else if (touchPos.px >= BTN_HPAD + BTN_WIDTH && touchPos.px < BTN_HPAD + 2 * BTN_WIDTH - 2) {
                    *hidden = !*hidden;
                } else if (touchPos.px >= BTN_HPAD + 2 * BTN_WIDTH && touchPos.px < BTN_HPAD + 3 * BTN_WIDTH - 2) {
                    shuffleWheelOptions(wheel);
                    (*shuffleHeld)++;
                }
            }
        }
    } else if (kHeld & KEY_TOUCH) {
        heldTime += 1;
        if (!*barHeld && !*hidden) {
            *scroll += prevTouch.py - touchPos.py;
        }
    } else if (kUp & KEY_TOUCH && heldTime < 30
            && (ABS(firstTouch.px-prevTouch.px)+ABS(firstTouch.py-prevTouch.py)) < 12
            && !*barHeld && !*hidden) {
        float x = prevTouch.px, y = prevTouch.py + *scroll - PAD - BORDER;
        int selectedOption = y / (HEIGHT + PAD);
        if (y > 0 && fmodf(y, HEIGHT + PAD) <= HEIGHT - 2 * BORDER && selectedOption < wheel->numOptions) {
            if (x >= PAD + BORDER && x <= BOTTOM_WIDTH - PAD - BORDER - HEIGHT) {
                SwkbdState swkbd;
                swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, MAX_OPTION_LEN - 1);
                swkbdSetInitialText(&swkbd, wheel->options[selectedOption]);

                static char buf[MAX_OPTION_LEN] = "";

                SwkbdButton button = swkbdInputText(&swkbd, buf, MAX_OPTION_LEN);
                if (button == SWKBD_BUTTON_CONFIRM) {
                    modifyWheelOption(wheel, selectedOption, buf);
                }
            } else if (x >= BOTTOM_WIDTH - PAD - HEIGHT && x <= BOTTOM_WIDTH - PAD - BORDER) {
                removeWheelOption(wheel, selectedOption);
            }
        }
    }

    if (kUp & KEY_TOUCH) {
        *barHeld = false;
    }

    prevTouch = touchPos;
}

int main() {
    srand(time(NULL));

    C3D_RenderTarget *top, *bottom;
    initGfx(&top, &bottom);
    initColors();
    initText();
    atexit(finish);

    Wheel wheel;
    initWheel(&wheel);
    fetchWheelOptions(&wheel);

    float scroll = 0.0f, maxScroll = 0.0f;
    int shuffleHeld = 0;
    bool barHeld = false, hidden = false;

    SwkbdState swkbd;
    swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, MAX_OPTION_LEN - 1);
    swkbdSetHintText(&swkbd, "Enter a new option");

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START) break;

        if (shuffleHeld > 0) {
            shuffleHeld++;
            if (shuffleHeld > 7) {
                shuffleHeld = 0;
            }
        }

        if (wheel.spinning) {
            updateWheel(&wheel);
        } else if (wheel.finishedSpin) {
            if (kDown & KEY_A) {
                wheel.finishedSpin = false;
            }
            if (kDown & KEY_X) {
                wheel.finishedSpin = false;
                removeWheelOption(&wheel, wheel.selectedOption);
            }
        } else {
            if (kDown & KEY_Y && wheel.numOptions < MAX_OPTIONS - 1) {
                SwkbdButton button = swkbdInputText(&swkbd, wheel.options[wheel.numOptions], MAX_OPTION_LEN);
                if (button == SWKBD_BUTTON_CONFIRM) {
                    addWheelOption(&wheel);
                }
            }

            if (kDown & KEY_A && wheel.numOptions > 0) {
                float angle = drand48() * 360.0f;
                spinWheelTo(&wheel, angle);
            }

            handleTouch(&wheel, &scroll, &barHeld, &hidden, &shuffleHeld);

            maxScroll = wheel.numOptions * (HEIGHT + PAD) - BOTTOM_HEIGHT + PAD + BAR_HEIGHT;
            scroll = MIN(scroll, maxScroll);
            scroll = MAX(scroll, 0.0f);
        }

        render(top, bottom, &wheel, scroll, maxScroll, hidden, shuffleHeld);
    }

    saveWheelOptions(&wheel);

    return 0;
}