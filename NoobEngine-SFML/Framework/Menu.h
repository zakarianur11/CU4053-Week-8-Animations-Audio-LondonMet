#pragma once
#include <SFML/Graphics.hpp>
#include "Input.h"
#include "BaseLevel.h"
#include "GameState.h"
#include "Collision.h"
#include "UI.h"
#include <string>
#include <iostream>
#include "../Level.h"


class Menu : public BaseLevel
{
public:
	Menu(sf::RenderWindow* hwnd, Input* in, GameState* game);
	~Menu();


	void handleInput(float dt) override;
	void update(float dt) override;
	void render() override;
	void MoveUp();
	void MoveDown();
	void updateVisualFeedback();
	int GetPressedItem() { return selectedItem; }


	sf::RenderWindow* window;
	Input* input;
	GameState* gameState;

	int selectedItem;
	sf::Font UIfont;
	sf::Font titleFont;

	sf::Text Title;
	UI UIText[2];
	




	sf::Sprite menu_sprite;
	sf::Texture menu_texture;

	sf::Vector2i MousePos;

	bool mouseOverAnyItem;
};

