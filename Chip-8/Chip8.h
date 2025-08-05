#pragma once

#include "defines.h"

#include <fstream>
#include <random>
#include <chrono>
#include <Windows.h>
#include <thread>


const unsigned int ROM_START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;
const unsigned int FRAMEBUF_WIDTH = 64;
const unsigned int FRAMEBUF_HEIGHT = 32;

struct Chip8 {
	
	
	Chip8();

	u8 memory[4096]{};

	// registers
	u8 V[16]{};

	u16 I{}; // index register: memory address pointer
	u16 pc{}; // program counter
	u8 sp{}; // points to topmost level of stack
	u8 delay_reg{}; // Decrement by 60Hz when non-zero
	u8 sound_reg{}; // Decrement by 60Hz when non-zero

	u16 stack[16]{};

	u32 framebuf[64 * 32]{};
	u8 fontset[FONTSET_SIZE] =
	{
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

	u8 keypad[16]{};

	u16 opcode;

	bool shouldPlaySound = false;

	std::default_random_engine randgen;
	std::uniform_int_distribution<int> randbyte;

	typedef void (Chip8::* Chip8Func)();
	Chip8Func table[0xF + 1];
	Chip8Func table0[0xE + 1];
	Chip8Func table8[0xE + 1];
	Chip8Func tableE[0xE + 1];
	Chip8Func tableF[0x65 + 1];

	void LoadROM(char const* filepath);

	void Table0()
	{
		((*this).*(table0[opcode & 0x000Fu]))();
	}

	void Table8()
	{
		((*this).*(table8[opcode & 0x000Fu]))();
	}

	void TableE()
	{
		((*this).*(tableE[opcode & 0x000Fu]))();
	}

	void TableF()
	{
		((*this).*(tableF[opcode & 0x00FFu]))();
	}

	void OP_NULL()
	{
	}

	void Cycle();

#pragma region OPCODES
	void OP_00E0() {
		memset(framebuf, 0, sizeof(framebuf));
	}

	void OP_00EE() {
		--sp;
		pc = stack[sp];
	}

	void OP_1nnn() {
		u16 address = opcode & 0x0FFFu;

		pc = address;
	}

	void OP_2nnn() {
		u16 address = opcode & 0xFFFu;

		stack[sp] = pc;
		++sp;
		pc = address;
	}

	void OP_3xkk() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 byte = opcode & 0x00FFu;

		if (V[x] == byte) {
			pc += 2;
		}
	}

	void OP_4xkk() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 byte = opcode & 0x00FFu;

		if (V[x] != byte) {
			pc += 2;
		}
	}

	void OP_5xy0() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 y = (opcode & 0x00F0u) >> 4u;

		if (V[x] == V[y]) {
			pc += 2;
		}
	}

	void OP_6xkk() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 byte = opcode & 0x00FFu;

		V[x] = byte;
	}

	void OP_7xkk() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 byte = opcode & 0x00FFu;

		V[x] += byte;
	}

	void OP_8xy0() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 y = (opcode & 0x00F0u) >> 4u;

		V[x] = V[y];
	}

	void OP_8xy1() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 y = (opcode & 0x00F0u) >> 4u;

		V[x] |= V[y];
		V[0xF] = 0;

	}

	void OP_8xy2() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 y = (opcode & 0x00F0u) >> 4u;

		V[x] &= V[y];
		V[0xF] = 0;

	}

	void OP_8xy3() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 y = (opcode & 0x00F0u) >> 4u;

		V[x] ^= V[y];
		V[0xF] = 0;

	}

	void OP_8xy4() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 y = (opcode & 0x00F0u) >> 4u;

		u16 sum = V[x] + V[y];

		if (sum > 255u) {
			V[0xF] = 1;
		}
		else {
			V[0xF] = 0;
		}

		V[x] = sum & 0xFFu;
	}

	void OP_8xy5() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 y = (opcode & 0x00F0u) >> 4u;

		if (V[x] > V[y])
			V[0xF] = 1;
		else
			V[0xF] = 0;

		V[x] -= V[y];
	}

	void OP_8xy6() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		
		V[0xF] = (V[x] & 0x1u);

		V[x] >>= 1;
 	}

	void OP_8xy7() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 y = (opcode & 0x00F0u) >> 4u;

		if (V[y] > V[x])
			V[0xF] = 1;
		else V[0xF] = 0;

		V[x] = V[y] - V[x];
		
	}

	void OP_8xyE() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		
		V[0xF] = (V[x] & 0x80u) >> 7u;
		
		V[x] <<= 1;
	}

	void OP_9xy0() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 y = (opcode & 0x00F0u) >> 4u;

		if (V[x] != V[y])
			pc += 2;
	}

	void OP_Annn() {
		u16 address = opcode & 0x0FFFu;

		I = address;
	}

	void OP_Bnnn() {
		u16 address = opcode & 0x0FFFu;

		pc = address + V[0];
	}

	void OP_Cxkk() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 byte = opcode & 0x00FFu;
		
		V[x] = randbyte(randgen) & byte;
	}

	void OP_Dxyn() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 y = (opcode & 0x00F0u) >> 4u;
		u8 n = opcode & 0x000Fu;

		u8 xPos = V[x] % FRAMEBUF_WIDTH;
		u8 yPos = V[y] % FRAMEBUF_HEIGHT;

		V[0xF] = 0;

		for (unsigned int row = 0; row < n; ++row) {
			u8 spriteByte = memory[I + row];
			for (unsigned int col = 0; col < 8; ++col) {
				u8 spritePixel = spriteByte & (0x80u >> col);
				u32* screenPixel = &framebuf[(yPos + row) * FRAMEBUF_WIDTH + (xPos + col)];

				if (spritePixel) {
					if (*screenPixel == 0xFFFFFFFF) {
						V[0xF] = 1;
					}

					*screenPixel ^= 0xFFFFFFFF;
				}
			}
		}
	}

	void OP_Ex9E() {
		u8 x = (opcode & 0x0F00u) >> 8u;

		u8 key = V[x];

		if (keypad[key]) {
			pc += 2;
		}
	}

	void OP_ExA1() {
		u8 x = (opcode & 0x0F00u) >> 8u;

		u8 key = V[x];

		if (!keypad[key]) {
			pc += 2;
		}
	}

	void OP_Fx07() {
		u8 x = (opcode & 0x0F00u) >> 8u;

		V[x] = delay_reg;
	}

	void OP_Fx0A() {
		u8 x = (opcode & 0x0F00u) >> 8u;

		if (keypad[0])
		{
			V[x] = 0;
		}
		else if (keypad[1])
		{
			V[x] = 1;
		}
		else if (keypad[2])
		{
			V[x] = 2;
		}
		else if (keypad[3])
		{
			V[x] = 3;
		}
		else if (keypad[4])
		{
			V[x] = 4;
		}
		else if (keypad[5])
		{
			V[x] = 5;
		}
		else if (keypad[6])
		{
			V[x] = 6;
		}
		else if (keypad[7])
		{
			V[x] = 7;
		}
		else if (keypad[8])
		{
			V[x] = 8;
		}
		else if (keypad[9])
		{
			V[x] = 9;
		}
		else if (keypad[10])
		{
			V[x] = 10;
		}
		else if (keypad[11])
		{
			V[x] = 11;
		}
		else if (keypad[12])
		{
			V[x] = 12;
		}
		else if (keypad[13])
		{
			V[x] = 13;
		}
		else if (keypad[14])
		{
			V[x] = 14;
		}
		else if (keypad[15])
		{
			V[x] = 15;
		}
		else
		{
			pc -= 2;
		}
	}

	void OP_Fx15() {
		u8 x = (opcode & 0x0F00u) >> 8u;

		delay_reg = V[x];
	}

	void OP_Fx18() {
		u8 x = (opcode & 0x0F00u) >> 8u;

		sound_reg = V[x];
	}
	
	void OP_Fx1E() {
		u8 x = (opcode & 0x0F00u) >> 8u;

		I += V[x];
	}

	void OP_Fx29() {
		u8 x = (opcode & 0x0F00u) >> 8u;

		I = FONTSET_START_ADDRESS + (5 * V[x]);
	}

	void OP_Fx33() {
		u8 x = (opcode & 0x0F00u) >> 8u;
		u8 value = V[x];

		memory[I + 2] = value % 10;
		value /= 10;

		memory[I + 1] = value % 10;
		value /= 10;

		memory[I] = value % 10;
	}

	void OP_Fx55() {
		u8 x = (opcode & 0x0F00u) >> 8u;

		for (unsigned int i = 0; i <= x; ++i) {
			memory[I + i] = V[i];
		}
	}

	void OP_Fx65() {
		u8 x = (opcode & 0x0F00u) >> 8u;

		for (unsigned int i = 0; i <= x; ++i) {
			V[i] = memory[I + i];
		}
	}
#pragma endregion
};


Chip8::Chip8() : randgen(std::chrono::system_clock::now().time_since_epoch().count()) {
	pc = ROM_START_ADDRESS;

	for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
		memory[FONTSET_START_ADDRESS + i] = fontset[i];
	}

	randbyte = std::uniform_int_distribution<int>(0, 255U);

	// Set up function pointer table
	table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::OP_1nnn;
	table[0x2] = &Chip8::OP_2nnn;
	table[0x3] = &Chip8::OP_3xkk;
	table[0x4] = &Chip8::OP_4xkk;
	table[0x5] = &Chip8::OP_5xy0;
	table[0x6] = &Chip8::OP_6xkk;
	table[0x7] = &Chip8::OP_7xkk;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::OP_9xy0;
	table[0xA] = &Chip8::OP_Annn;
	table[0xB] = &Chip8::OP_Bnnn;
	table[0xC] = &Chip8::OP_Cxkk;
	table[0xD] = &Chip8::OP_Dxyn;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

	for (size_t i = 0; i <= 0xE; i++)
	{
		table0[i] = &Chip8::OP_NULL;
		table8[i] = &Chip8::OP_NULL;
		tableE[i] = &Chip8::OP_NULL;
	}

	table0[0x0] = &Chip8::OP_00E0;
	table0[0xE] = &Chip8::OP_00EE;

	table8[0x0] = &Chip8::OP_8xy0;
	table8[0x1] = &Chip8::OP_8xy1;
	table8[0x2] = &Chip8::OP_8xy2;
	table8[0x3] = &Chip8::OP_8xy3;
	table8[0x4] = &Chip8::OP_8xy4;
	table8[0x5] = &Chip8::OP_8xy5;
	table8[0x6] = &Chip8::OP_8xy6;
	table8[0x7] = &Chip8::OP_8xy7;
	table8[0xE] = &Chip8::OP_8xyE;

	tableE[0x1] = &Chip8::OP_ExA1;
	tableE[0xE] = &Chip8::OP_Ex9E;

	for (size_t i = 0; i <= 0x65; i++)
	{
		tableF[i] = &Chip8::OP_NULL;
	}

	tableF[0x07] = &Chip8::OP_Fx07;
	tableF[0x0A] = &Chip8::OP_Fx0A;
	tableF[0x15] = &Chip8::OP_Fx15;
	tableF[0x18] = &Chip8::OP_Fx18;
	tableF[0x1E] = &Chip8::OP_Fx1E;
	tableF[0x29] = &Chip8::OP_Fx29;
	tableF[0x33] = &Chip8::OP_Fx33;
	tableF[0x55] = &Chip8::OP_Fx55;
	tableF[0x65] = &Chip8::OP_Fx65;
}

void Chip8::LoadROM(char const* filepath) {
	std::ifstream file(filepath, std::ios::binary | std::ios::ate);

	if (file.is_open()) {
		std::streampos size = file.tellg();
		char* buffer = new char[size];

		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		for (long i = 0; i < size; ++i) {
			memory[ROM_START_ADDRESS + i] = buffer[i];
		}

		delete[] buffer;
	}
}

void PlayBeep() {
	Beep(440, 100);

}

void Chip8::Cycle() {
	opcode = (memory[pc] << 8u) | memory[pc + 1];

	pc += 2;

	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

	if (delay_reg > 0)
		--delay_reg;

	static bool sound_playing = false;
	if (sound_reg > 0) {
		if (!sound_playing) {
			sound_playing = true;

			std::thread(PlayBeep).detach();
		}
		--sound_reg;
	}
	else {
		sound_playing = false;
	}

}

