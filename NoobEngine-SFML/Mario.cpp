#include "Mario.h"

Mario::Mario()
{
	marioSpriteSheet.loadFromFile("gfx/MarioSheetT.png");
	setSize(sf::Vector2f(15 * 4, 21 * 4));
	setPosition(400, 0);
	setTexture(&marioSpriteSheet);

	Walk.addFrame(sf::IntRect(0, 0, 15, 21));
	Walk.addFrame(sf::IntRect(15, 0, 15, 21));
	Walk.addFrame(sf::IntRect(30, 0, 15, 21));
	Walk.addFrame(sf::IntRect(45, 0, 15, 21));
	Walk.setFrameSpeed(1.f / 10.f);

	Swim.addFrame(sf::IntRect(0, 21, 16, 20));
	Swim.addFrame(sf::IntRect(16, 21, 16, 20));
	Swim.addFrame(sf::IntRect(32, 21, 16, 20));
	Swim.setFrameSpeed(1.f / 2.4);

	Duck.addFrame(sf::IntRect(0, 44, 17, 17));
	//Duck.addFrame(sf::IntRect(17, 44, 17, 17));
	Duck.setFrameSpeed(1.f / 2.f);
	currentAnimation = &Walk;

}

void Mario::handleInput(float dt)
{
	velocity.x = 0.f;
	setTextureRect(currentAnimation->getCurrentFrame());
	if (input->isKeyDown(sf::Keyboard::Left))
	{
		velocity.x = -200;
		currentAnimation = &Walk;
		currentAnimation->setFlipped(true);
	}
	else if (input->isKeyDown(sf::Keyboard::Right))
	{
		velocity.x = 200;
		currentAnimation = &Walk;
		currentAnimation->setFlipped(false);
	}
	else if (input->isKeyDown(sf::Keyboard::Down))
	{
		currentAnimation = &Duck;
	}
	else if (input->isKeyDown(sf::Keyboard::Up))

	{
		currentAnimation = &Swim;
	}

	else
	{
		currentAnimation->reset();
		currentAnimation = &Walk;
	}
	if (input->isKeyDown(sf::Keyboard::Space) && canJump)
	{
		Jump(160.f);
	}
	currentAnimation->animate(dt);
	
}
