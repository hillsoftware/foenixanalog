#define F256LIB_IMPLEMENTATION
#include "f256lib.h"
#include "f256.h"

// Keyboard
#define KEYREADY 0xDDC1

// Analog joystick registers 
#define JOY1X    0xDDEA
#define JOY1Y    0xDDEB
#define JOY2X    0xDDEC
#define JOY2Y    0xDDED

// Joystick button registers
#define JOY1B    0xDC01
#define JOY2B    0xDC00

// Active-low button masks 
#define BUTTON_1 0x04
#define BUTTON_2 0x08

static void displayJoystick(uint8_t row, uint8_t x, uint8_t y, uint8_t buttons) {
    textGotoXY(7, row);
    printf("%02X", x);

    textGotoXY(12, row);
    printf("%02X", y);

    textGotoXY(18, row);
    putchar((buttons & BUTTON_1) ? '0' : '1');

    textGotoXY(23, row);
    putchar((buttons & BUTTON_2) ? '0' : '1');
}

int main(int argc, char *argv[])
{
    uint8_t x1, y1, x2, y2;
    uint8_t buttons1, buttons2;

    //SET_IO_PAGE(PAGE_0);

    textClear();

    printf("Joy1 X:   Y:   B1:  B2:\n");
    printf("Joy2 X:   Y:   B1:  B2:\n");

    for (;;) {
        //Exit when a key is hit
        if (!(PEEK(KEYREADY) & 0x01)) break;

        // Read analog values
        x1 = PEEK(JOY1X);
        y1 = PEEK(JOY1Y);
        x2 = PEEK(JOY2X);
        y2 = PEEK(JOY2Y);

        // Read button registers
        buttons1 = PEEK(JOY1B);
        buttons2 = PEEK(JOY2B);

        // Update values
        displayJoystick(0, x1, y1, buttons1);
        displayJoystick(1, x2, y2, buttons2);
    }
    return 0;
}

