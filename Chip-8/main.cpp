#include <chrono>
#include <iostream>
#include <string>

#include "Platform.h"
#include "Chip8.h"

int main(int argc, char* argv[]) {

	if (argc != 4) {
		std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
		std::exit(EXIT_FAILURE);
	}

	int videoScale = std::stoi(argv[1]);
	int cycleDelay = std::stoi(argv[2]);
	const char* romFilename = argv[3];

	Platform platform("CHIP-8 Emulator", FRAMEBUF_WIDTH * videoScale, FRAMEBUF_HEIGHT * videoScale, FRAMEBUF_WIDTH, FRAMEBUF_HEIGHT);

	Chip8 chip8;
	chip8.LoadROM(romFilename);

	int videoPitch = sizeof(chip8.framebuf[0]) * FRAMEBUF_WIDTH;

	auto lastCycleTime = std::chrono::high_resolution_clock::now();
	bool quit = false;

	while (!quit) {
		quit = platform.ProcessInput(chip8.keypad);

		auto currentTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

		if (dt > cycleDelay) {
			lastCycleTime = currentTime;

			chip8.Cycle();

			platform.Update(chip8.framebuf, videoPitch);
		}
	}

	return 0;
}