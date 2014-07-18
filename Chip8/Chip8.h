#pragma once
#include <iostream>
#include <fstream>
#include <string>

#define UPSCALE 10
#define SPEED 60 // clock cycles a second

/*
Keypad                   Keyboard
+-+-+-+-+                +-+-+-+-+
|1|2|3|C|                |1|2|3|4|
+-+-+-+-+                +-+-+-+-+
|4|5|6|D|                |Q|W|E|R|
+-+-+-+-+       =>       +-+-+-+-+
|7|8|9|E|                |A|S|D|F|
+-+-+-+-+                +-+-+-+-+
|A|0|B|F|                |Z|X|C|V|
+-+-+-+-+                +-+-+-+-+
*/

// http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
class Chip8 {

public:
	Chip8();
	~Chip8();

	// load a game into memory
	void loadGame(std::string gameName);

	// Emulates a cycle, should be called 60 times a second
	void cycle();
	void decClocks();

	// Check if the display needs updating
	bool getNeedRedraw();
	// Set the display needs updating flag
	void setNeedRedraw(bool set);
	// Returns an array of width * height describing the display state
	const unsigned char * getGraphics();
	// Returns the width of the display
	unsigned int getWidth();
	// Returns the height of the display
	unsigned int getHeight();

	// Sets whether a key is pressed or not
	void setKeyState(unsigned int key, bool state);

private:

	// Called by constructor, sets defualts
	void init();

	// The Chip 8 has 35 opcodes which are all two bytes long
	unsigned short opcode;

	// The Chip 8 has 4K memory in total
	unsigned char memory[4096];
	
	// CPU registers: The Chip 8 has 15 8-bit general purpose registers named V0,V1 up to VE. The 16th register is used  for the ‘carry flag’.
	unsigned char V[16];
	
	// There is an Index register I and a program counter (pc) which can have a value from 0x000 to 0xFFF (0 to 4095)
	unsigned short I;
	unsigned short pc;

	/*
	Memory Map
	----------
	0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
	0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
	0x200-0xFFF - Program ROM and work RAM

	The Graphics System:
	--------------------
	The chip 8 has one instruction that draws sprite to the screen. 
	Drawing is done in XOR mode and if a pixel is turned off as a result of drawing, the VF register is set. This is used for collision detection.

	The graphics of the Chip 8 are black and white and the screen has a total of 2048 pixels (64 x 32). 
	This can easily be implemented using an array that hold the pixel state (1 or 0).

	*/

	unsigned int height;
	unsigned int width;
	unsigned char * gfx;
	bool needsRedraw;

	// Interupts and hardware registers. 
	// The Chip 8 has none, but there are two timer registers that count at 60 Hz. 
	// When set above zero they will count down to zero.
	unsigned char delay_timer;
	unsigned char sound_timer;

	/*
	It is important to know that the Chip 8 instruction set has opcodes that allow the program to jump to a certain address or call a subroutine. 
	While the specification don’t mention a stack, you will need to implement one as part of the interpreter yourself. 
	The stack is used to remember the current location before a jump is performed. So anytime you perform a jump or call a subroutine, store the program counter in the stack before proceeding. 
	The system has 16 levels of stack and in order to remember which level of the stack is used, you need to implement a stack pointer (sp).
	*/
	unsigned short stack[16];
	unsigned short sp;

	//Finally, the Chip 8 has a HEX based keypad (0x0-0xF), you can use an array to store the current state of the key.
	bool key[16];

};