// TDGame.cpp : Defines the entry point to the NintendoSDK application.
//

#include "TDpch.h"
#include "Game.h"

#if defined(PLATFORM_SWITCH)
//Switch Code Emitted
#elif defined(PLATFORM_WINDOWS) 
int main(int argc, char* argv[])
#endif
{
	std::cout << "It's alive!" << std::endl;
	std::unique_ptr<TDGame::Game> game = std::make_unique<TDGame::Game>();
}
