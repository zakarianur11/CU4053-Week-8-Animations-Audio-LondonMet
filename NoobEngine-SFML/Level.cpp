#include "Level.h"
Level::Level(sf::RenderWindow* hwnd, Input* in, GameState* gs,sf::View* v, World* w, TileManager* tm)
{
	window = hwnd;
	input = in;
	gameState = gs;
	view = v;
	world = w;	
	tileManager = tm;
	audioManager = new AudioManager();

	world->AddGameObject(zomb);
	world->AddGameObject(mario);
	
	zomb.setInput(in);
	mario.setInput(in);

}

Level::~Level()
{
	//Making pointers null
	window = nullptr;
	input = nullptr;
	gameState = nullptr;
	view = nullptr;
	world = nullptr;
	tileManager = nullptr;
	if (audioManager != nullptr) {
		delete audioManager;
		audioManager = nullptr;
	}
}

void Level::handleInput(float dt)
{
	if (input->isKeyDown(sf::Keyboard::Escape))
	{
		exit(0);
	}
	if (input->isKeyDown(sf::Keyboard::Tab))
	{
		input->setKeyUp(sf::Keyboard::Tab);
		gameState->setCurrentState(State::TILEEDITOR);
	}

	zomb.handleInput(dt);
	mario.handleInput(dt);
}

// Update game objects
void Level::update(float dt)
{
	zomb.update(dt);
	//Move the view to follow the player
	//view->setCenter(view->getCenter().x, 360);
	//
	//sf::Vector2f playerPosition = player.getPosition();
	//float newX = std::max(playerPosition.x, view->getSize().x / 2.0f);
	//view->setCenter(newX, view->getCenter().y);
	//window->setView(*view);
}

// Render level
void Level::render()
{
	
	window->draw(zomb);
	window->draw(mario);


	if (gameState->getCurrentState() == State::LEVEL)
	{
		tileManager->render(false);
	}

}


void Level::adjustViewToWindowSize(unsigned int width, unsigned int height) 
{
	sf::FloatRect visibleArea(0, 0, static_cast<float>(width), static_cast<float>(height));
	view->setSize(static_cast<float>(width), static_cast<float>(height));
	view->setCenter(static_cast<float>(width) / 2, static_cast<float>(height) / 2);
}