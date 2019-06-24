
#pragma once
#include <string>
class VanguardClientUnmanaged
{
public:
	static void CORE_STEP();
	static void LOAD_GAME_START(std::string romPath);
	static void LOAD_GAME_DONE();
	static void GAME_CLOSED();
	static void CLOSE_GAME();
	static int GAME_NAME;
	static void InvokeEmuThread();
};