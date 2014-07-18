#include <SFML/Graphics.hpp>
#include <sstream>
#include "Chip8.h"

void drawScreen(unsigned int width, unsigned int height, const unsigned char * gfx, sf::RenderWindow * window);
void debugOutput(const unsigned char * gfx, unsigned int width, unsigned int height);
void updateKeystate(Chip8  & chip);

int main(int argc, char ** argv) {
	bool stepMode = false;
	bool step = false;
	bool fastmode = false;
	Chip8 chip8;
	if(argc < 2) {
		std::cout << "No game argument given!" << std::endl;
		// for testing load a file anyway
		chip8.loadGame("Trip8 Demo (2008) [Revival Studios].ch8");
	}
	else {
		chip8.loadGame(argv[1]);
	}
	sf::RenderWindow * window = new sf::RenderWindow(sf::VideoMode(chip8.getWidth() * UPSCALE, chip8.getHeight() * UPSCALE), "Chip 8 Emulator");
	window->setFramerateLimit(60);

	static float refreshSpeed= 1.f/60.f;
	sf::Clock clock;
	const unsigned char * gfx = nullptr;
    while(window->isOpen()) {
        sf::Event event;
        while (window->pollEvent(event)) {
            if(event.type == sf::Event::Closed)
                window->close();
			else if (event.type == sf::Event::KeyReleased) {
				if(event.key.code == sf::Keyboard::Equal) {
					debugOutput(chip8.getGraphics(), chip8.getWidth(), chip8.getHeight());
				}
				else if(event.key.code == sf::Keyboard::Num0) {
					stepMode = !stepMode;
				}
				else if(event.key.code == sf::Keyboard::N && stepMode) {
					step = true;
				}
				else if(event.key.code == sf::Keyboard::G) {
					fastmode = !fastmode;
				}
			}
        }

		if( (!stepMode && clock.getElapsedTime().asSeconds() >= refreshSpeed) || (stepMode && step) || fastmode ) {
			updateKeystate(chip8);
			chip8.cycle();
			if(chip8.getNeedRedraw()) {
				window->clear();
				// draw
				gfx = chip8.getGraphics();
				drawScreen(chip8.getWidth(), chip8.getHeight(), gfx, window);
				window->display();
				chip8.setNeedRedraw(false);
			}
			clock.restart();
			if(stepMode) {
				step = false;
			}
		}
    }
	gfx = nullptr;
	delete window;
	window = nullptr;

    return 0;
}

void drawScreen(unsigned int width, unsigned int height, const unsigned char * gfx, sf::RenderWindow * window) {
	sf::RectangleShape rectangle(sf::Vector2f(UPSCALE, UPSCALE));
	rectangle.setFillColor(sf::Color::White);
	for(unsigned int y = 0; y < height; y ++) {
		for(unsigned int x = 0; x < width; x ++) {
			unsigned char state = gfx[x + (width*y)];
			if(state > 0) {
				rectangle.setPosition( (float) (x * UPSCALE), (float) (y * UPSCALE));
				window->draw(rectangle);
			}
		}
	}


}

void debugOutput(const unsigned char * gfx, unsigned int width, unsigned int height) {
	std::stringstream ss;
	for(unsigned int y = 0; y < height; y ++) {
		for(unsigned int x = 0; x < width; x ++) {
			unsigned char state = gfx[x + (width*y)];
			ss << (state ? '#' : ' ');
			if(x == width-1) {
				ss << '\n';
			}
		}
	}
	std::cout << ss.str();
}


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

void updateKeystate(Chip8  & chip) {
	chip.setKeyState(0x1, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num1)); // 1
	chip.setKeyState(0x2, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num2)); // 2
	chip.setKeyState(0x3, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num3)); // 3
	chip.setKeyState(0xC, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Num4)); // C

	chip.setKeyState(0x4, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)); // 4
	chip.setKeyState(0x5, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)); // 5
	chip.setKeyState(0x6, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E)); // 6
	chip.setKeyState(0xD, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)); // D

	chip.setKeyState(0x7, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)); // 7
	chip.setKeyState(0x8, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)); // 8
	chip.setKeyState(0x9, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)); // 9
	chip.setKeyState(0xE, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F)); // E

	chip.setKeyState(0xA, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z)); // A
	chip.setKeyState(0x0, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X)); // 0
	chip.setKeyState(0xB, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C)); // B
	chip.setKeyState(0xF, sf::Keyboard::isKeyPressed(sf::Keyboard::Key::V)); // F
}