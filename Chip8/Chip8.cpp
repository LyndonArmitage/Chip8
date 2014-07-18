#include <cstdlib>
#include <ctime>
#include "Chip8.h"

unsigned char chip8_fontset[80] = { 
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8() {
	init();
}

Chip8::~Chip8() {
	if(gfx != nullptr) {
		delete [] gfx;
		gfx = nullptr;
	}
}

void Chip8::init() {
	opcode = 0;
	
	for(unsigned int i = 0; i < 4096; i ++) {
		if(i < 80) {
			memory[i] = chip8_fontset[i];
		}
		else {
			memory[i] = 0;
		}
	}

	for(unsigned int i = 0; i < 16; i ++) {
		V[i] = 0;
	}

	I = 0;
	pc = 0x200;

	// Graphics stuff
	width = 64;
	height = 32;
	gfx = new unsigned char [width * height];
	for(unsigned int i = 0; i < width * height; i ++) {
		gfx[i] = 0;
	}
	needsRedraw = true;

	delay_timer = 0;
	sound_timer = 0;
	
	for(unsigned int i = 0; i < 16; i ++) {
		stack[i] = 0;
	}

	sp = 0;

	for(unsigned int i = 0; i < 16; i ++) {
		key[i] = false;
	}

	srand( (unsigned int) time(NULL) ); // see RNG with the time
}

void Chip8::loadGame(std::string gameName) {
	char * rom = nullptr; // we will store the rom in a temporary area
	unsigned long size = 0;
	static unsigned int startPos = 0x200; // programs start at 0x200 in Chip8 normally
	std::ifstream input(gameName, std::ios::binary);

	if(input && input.is_open()) {
		input.seekg(0, std::ios::end); // fast forward to end of stream/file
		size = (unsigned long) input.tellg(); // record pos (so we can find out the size of the file)
		if(size + startPos >= 4096) {
			// Check if loading the program will result in us overflowing the memory
			std::cout << "Error: " << gameName << " is to large to load into memory" << std::endl;
		}
		else {
			input.seekg(0, std::ios::beg); // rewind back to the start
			rom = new char[size+1]; // allocate some memory to load the rom into
			input.read(rom, size); // read it into the array
		}
		input.close();
	}
	else {
		std::cout << "Error: problem opening " << gameName << std::endl;
	}

	if(rom != nullptr) {
		for(unsigned int i = startPos; i < 4096 && i-startPos < size; i ++) {
			// fill up the memory with the rom
			memory[i] = (unsigned char) rom[i - startPos];
		}
		std::cout << "Loaded " << gameName << std::endl;
		delete [] rom; // deallocate the memory
		rom = nullptr;
	}
	
}

void Chip8::cycle() {
	opcode = memory[pc] << 8 | memory[pc + 1]; // fetch

	//std::cout.setf(std::ios::hex, std::ios::basefield);
	//std::cout << "pc: 0x" << pc << " opcode: 0x" << opcode << std::endl;
	//std::cout.unsetf(std::ios::hex);

	//TODO: Add Chip48/SuperChip8 opcodes!

	// decode and execute
	switch(opcode & 0xF000) {
	
	case 0x0000 :
		// 0x0??? opcodes

		switch(opcode & 0x00F0) {
		case 0x00C0 :
			// 0x00CN SCD nibble
			// Scroll down N lines
			// When in Chip8 mode scrolls down N/2 lines, when in SuperChip mode scroll down N lines
			// TODO
			pc += 2;
			break;
		}

		switch(opcode & 0x00FF) {

		case 0x00E0:
			// 0x00E0 CLS
			// Clear screen
			for(unsigned int i = 0; i < width * height; i ++) {
				gfx[i] = 0;
			}
			needsRedraw = true;
			pc += 2; // increment counter
			break;

		case 0x00EE:
			// 0x00EE RET
			// Return from a subroutine
			sp --; // decrement stack pointer
			pc = stack[sp]; // set the program counter to the old position
			pc += 2; // increment
			break;

		case 0x00FB:
			// 0x00FB SCR
			// Scroll 4 pixels right in SuperChip mode, or 2 pixels in Chip8 mode
			// TODO
			pc += 2; // increment
			break;

		case 0x00FC:
			// 0x00FB SCL
			// Scroll 4 pixels left in SuperChip mode, or 2 pixels in Chip8 mode
			// TODO
			pc += 2; // increment
			break;

		case 0x00FD:
			// 0x00FD EXIT
			// Exit the emulator
			// TODO
			std::cout << "Exit!" << std::endl;
			break;

		case 0x00FE:
			// 0x00FE LOW
			// Set emulator to normal Chip8 resolution, 64*32
			// TODO
			pc += 2;
			break;

		case 0x00FF:
			// 0x00FF HIGH
			// Set emulator to SuperChip resolution, 128*64
			// TODO
			pc += 2;
			break;

		default:
			std::cout.setf(std::ios::hex, std::ios::basefield);
			std::cout << "Unknown 0x00?? opcode: " << opcode << std::endl;
			std::cout.unsetf(std::ios::hex);
			//pc += 2;
			break;
		}
		break;

	case 0x1000:
		// 0x1NNN JP addr
		// Set the PC to location NNN
		pc = opcode & 0x0FFF;
		break;

	case 0x2000:
		// 0x2NNN CALL addr
		// Call the subroutine at NNN
		stack[sp] = pc; // store the current position on the stack
		sp ++; // increment the stack pointer
		pc = opcode & 0x0FFF;
		break;

	case 0x3000:
		// 0x3XKK SE Vx, byte
		// Skip next instruction if Vx = KK
		if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
			// >> 8 shifts the result 8 bits right, making it a 1 byte value
			// KK doesn't need shifting since it is already on the right
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;

	case 0x4000:
		// 0x4XKK SNE Vx, byte
		// Skip next instruction if Vx != KK
		if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	
	case 0x5000:
		// 0x5XY0 SE Vx, Vy
		// Skip next instruction if Vx = Vy
		if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;
	
	case 0x6000:
		// 0x6XKK LD Vx, byte
		// Set Vx = KK
		V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		pc += 2;
		break;

	case 0x7000:
		// 0x7XKK ADD Vx, byte
		// Set Vx = Vx + KK
		V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		pc += 2;
		break;

	case 0x8000:
		// 0x8??? opcodes
		switch(opcode & 0x000F) {

		case 0x0000:
			// 0x8XY0 LD Vx, Vy
			// Set Vx to value of Vy
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0001:
			// 0x8XY1 OR Vx, Vy
			// Set Vx = Vx OR Vy
			// Bitwise OR |= is shorthand
			V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0002:
			// 0x8XY2 AND Vx, Vy
			// Set Vx = Vx AND Vy
			// Bitwise AND &= is shorthand
			V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0003:
			// 0x8XY3 XOR Vx, Vy
			// Set Vx = Vx XOR Vy
			// Bitwise XOR ^= is shorthand
			V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0004:
			// 0x8XY4 ADD Vx, Vy
			// Set Vx = Vx + Vy, set Vf = carry
			// Add the two together and store result in Vx, and mark whether there was a carry or not in Vf
			if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) {
				// if ( Vy > (255 - Vx) )
				V[0xF] = 1; // There is a carry
			}
			else {
				V[0xF] = 0;
			}
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4]; // set the value
			pc += 2;
			break;

		case 0x0005:
			// 0x8XY5 SUB Vx, Vy
			// Set Vx = Vx - Vy, set Vf = NOT borrow
			// Subtract Vy from Vx, setting Vf to 1 if Vx > vY or 0 otherwise
			if(V[(opcode & 0x00F0) >> 4] > (V[(opcode & 0x0F00) >> 8])) {
				V[0xF] = 0; // There is a borrow
			}
			else {
				V[0xF] = 1;
			}
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4]; // set the value
			pc += 2;
			break;

		case 0x0006:
			// 0x8XY6 SHR Vx {, Vy}
			// Shift Vx to the right. Setting Vf to 1 if the least signficant bit is a 1 else setting it to 0
			
			V[0xF] = V[ (opcode & 0x0F00) >> 8] & 0x1;

			// shorthand for shifting and setting
			V[(opcode & 0x0F00) >> 8] >>= 1;  // set the value
			pc += 2;
			break;

		case 0x0007:
			// 0x8XY7 SUBN Vx, Vy
			// Set Vx = Vy - Vx, set Vf = NOT borrow
			// Subtract Vx from Vy, setting Vf to 1 if Vy > Vx or 0 otherwise
			if(V[(opcode & 0x00F0) >> 8] > (V[(opcode & 0x0F00) >> 4])) {
				V[0xF] = 0; // There is a borrow
			}
			else {
				V[0xF] = 1;
			}
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x000E:
			// 0x8XYE SHL Vx {, Vy}
			// Shift Vx to the left. Setting Vf to 1 if the most signficant bit is a 1 else setting it to 0
			
			V[0xF] = V[ (opcode & 0x0F00) >> 8] >> 7;

			// shorthand for shifting and setting
			V[(opcode & 0x0F00) >> 8] <<= 1;  // set the value
			pc += 2;
			break;

		default:
			std::cout.setf(std::ios::hex, std::ios::basefield);
			std::cout << "Unknown 0x8??? opcode: " << opcode << std::endl;
			std::cout.unsetf(std::ios::hex);
			//pc += 2;
			break;
		}

		break;

	case 0x9000:
		// 0x9XY0 SNE Vx, Vy
		// Skip next instruction if Vx != Vy
		if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;

	case 0xA000:
		// 0xANNN LD I, addr
		// Set I = NNN
		I = opcode & 0x0FFF;
		pc += 2;
		break;

	case 0xB000:
		// 0xBNNN JP V0, addr
		// Set the PC to NNN + V0
		pc = (opcode & 0x0FFF) + V[0x0];
		break;

	case 0xC000:
		// 0xCXNN RND Vx, byte
		// Set Vx = (random number between 0 - 255) AND (NNN)
		V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
		pc += 2;
		break;

	case 0xD000: {
		// Draw opcode
		// 0xDXYN DRW Vx, Vy, nibble

		// TODO: Add support for 8*16 and 16*16 sprites when using height of 0 (for Chip8 and SuperChip)

		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		V[0xF] = 0; // set Vf to 0, will be set to 1 if any collisions occur

		unsigned short pixel;
		// for each row of the sprite
		for(unsigned int yline = 0; yline < height; yline++) {
			pixel = memory[I + yline]; // set whether we should draw a pixel or not

			// for each pixel in the row
			for(unsigned int xline = 0; xline < 8; xline++) {

				// Check if there is a pixel that needs drawing present at that x and y in the sprite
				if((pixel & (0x80 >> xline)) != 0) {

					// check if there is a sprite already there in our graphics
					if(gfx[(x + xline + ((y + yline) * width))] == 1) {
						V[0xF] = 1; // if there is set the flag
					}

					gfx[x + xline + ((y + yline) * width)] ^= 1; // XOR onto the screen
				}
			}
		}
		

		needsRedraw = true; // set the flag to tell the emulator to redraw

		pc += 2;
		break;}

	case 0xE000:
		// 0xE??? opcodes
		switch(opcode & 0x00FF) {

		case 0x009E:
			// 0xEX9E SKP Vx
			// Skip next opcode if key with value of Vx is pressed
			if(key[V[(opcode & 0x0F00) >> 8]]) {
				pc += 4; // skip
			}
			else {
				pc += 2;
			}
			break;

		case 0x00A1:
			// 0xEXA1 SKNP Vx
			// Skip next opcode if key with value of Vx is not pressed
			if(!key[V[(opcode & 0x0F00) >> 8]]) {
				pc += 4; // skip
			}
			else {
				pc += 2;
			}
			break;

		default :
			std::cout.setf(std::ios::hex, std::ios::basefield);
			std::cout << "Unknown 0xE??? opcode: " << opcode << std::endl;
			std::cout.unsetf(std::ios::hex);
			//pc += 2;
			break;
		}
		
		break;

	case 0xF000:
		// 0xF??? opcodes
		switch(opcode & 0x00FF) {
		
		case 0x0007:
			// 0xFX07 LD Vx, DT
			// Set Vx = The delay timer
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

		case 0x000A: {
			// 0xFX0A LD Vx, K
			// Wait for keypress, then store in Vx
			bool keyPressed = false;
			for(unsigned int i = 0; i < 16; i ++) {
				if(key[i]) {
					keyPressed = true;
					V[(opcode & 0x0F00) >> 8] = i;
					break;
				}
			}
			if(!keyPressed) {
				return; // finish this instruction, essentially causing it to wait since pc hasnt been incremented
			}
			pc += 2;
			break;}

		case 0x0015:
			// 0xFX15 LD DT, Vx
			// Set the delay timer = Vx
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0018:
			// 0xFX18 LD ST, Vx
			// Set the sound timer = Vx
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x001E:
			// 0xFX1E ADD I, Vx
			// Set I = I + Vx

			if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF) {	// VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
				V[0xF] = 1;
			}
			else {
				V[0xF] = 0;
			}

			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;

			// Not sure if this should set the carry or not, I don't think it should since it's manipulating the I register which is special.
			break;

		case 0x0029:
			// 0xFX29 LD F, Vx
			// Set I = location of sprite for value of Vx
			I = V[(opcode & 0x0F00) >> 8] * 0x5; // sprites are 8*5
			pc += 2;
			break;

		case 0x0030:
			// 0xFX30 LD HF, Vx
			// Set I = location of SuperChip sprite for value of Vx
			// TODO
			//I = V[(opcode & 0x0F00) >> 8] * 0xA; // sprites are 8*10
			pc += 2;
			break;

		case 0x0033:
			// 0xFX33 LD B, Vx
			// Store the BCD representations of the value of Vx in memory locations I, I+1, and I+2
			
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;

			pc += 2;
			break;

		case 0x0055:
			// 0xFX55 LD [I], Vx
			// Store registers V0 through Vx in memory starting at location I
			for(unsigned int i = 0; i <= ((opcode & 0x0F00) >> 8); i ++) {
				memory[I + i] = V[i];
			}

			// not sure on this line as it was found in an example emulator but the doc I have doesn't mention incrementing I
			I += ((opcode & 0x0F00) >> 8) + 1; // increment I so it's pointing after all the newly inserted values
			
			pc += 2;
			break;

		case 0x0065:
			// 0xFX65 LD Vx, [I]
			// Read values from memory into registers starting at I, going through Vx registers
			for(unsigned int i = 0; i <= ((opcode & 0x0F00) >> 8); i ++) {
				V[i] = memory[I + i];
			}
			
			// not sure on this line as it was found in an example emulator but the doc I have doesn't mention incrementing I
			I += ((opcode & 0x0F00) >> 8) + 1; // increment I so it's pointing after all the newly inserted values
			
			pc += 2;
			break;

		case 0x0075:
			// 0xFX75 LD R, Vx
			// HP48 Save Flag
			// TODO
			pc += 2;
			break;

		case 0x0085:
			// 0xFX75 LD Vx, R
			// HP48 Load Flag
			// TODO
			pc += 2;
			break;

		default :
			std::cout.setf(std::ios::hex, std::ios::basefield);
			std::cout << "Unknown 0xF??? opcode: " << opcode << std::endl;
			std::cout.unsetf(std::ios::hex);
			//pc += 2;
			break;
		}

		break;


	default:
		std::cout.setf(std::ios::hex, std::ios::basefield);
		std::cout << "Unknown opcode: " << opcode << std::endl;
		std::cout.unsetf(std::ios::hex);
		//pc += 2;
		break;
	
	}

	if(delay_timer > 0) delay_timer --;
	if(sound_timer > 0) sound_timer --;
}


// Graphics stuff

bool Chip8::getNeedRedraw() {
	return this->needsRedraw;
}

void Chip8::setNeedRedraw(bool set) {
	this->needsRedraw = set;
}

const unsigned char * Chip8::getGraphics() {
	return gfx;
}

unsigned int Chip8::getWidth() {
	return this->width;
}

unsigned int Chip8::getHeight() {
	return this->height;
}


void Chip8::setKeyState(unsigned int key, bool state) {
	this->key[key] = state;
}