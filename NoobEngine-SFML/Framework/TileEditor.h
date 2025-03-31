#pragma once
#include <SFML/Graphics.hpp>
#include "BaseLevel.h"
#include "Input.h"
#include "GameState.h"
#include "World.h"
#include "TileManager.h"
#include <string>
#include <iostream>

class TileEditor
{
public:
	TileEditor(sf::RenderWindow* hwnd, Input* in, GameState* game, sf::View* view, World* w, TileManager* tm);
	~TileEditor();

	void handleInput(float dt);
	void update(float dt);
	void render();
	void moveView(float dt);
	TileManager* getTileManager() { return tileManager; }
private:
	// Default variables for level class.
	sf::RenderWindow* window;
	sf::View* view;
	Input* input;
	GameState* gameState;
	World* world;

	TileManager* tileManager;
	sf::Font font;
	
	sf::Vector2f originalViewSize;

	//moving the view variables
	float currentZoomLevel = 1.0f;
	sf::Vector2i lastMousePos;
	bool isDragging = false;
	sf::Texture mouseCurosorTex;
	sf::Sprite mouseCurosorGrab;


};

