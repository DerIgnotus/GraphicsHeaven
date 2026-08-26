#include "Game.hpp"

int main()
{
	Game game(1600, 900, "Graphics Heaven", false);
	if (game.ExtInit())
		return 1;
	if (game.Init())
		return 1;

	while (game.GetGameState() != GameState::GAME_END && !glfwWindowShouldClose(game.GetWindow())) {
		game.Run();
	}

}