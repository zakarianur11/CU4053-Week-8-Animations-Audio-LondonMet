#pragma once
#include "Framework/GameObject.h"
#include "Framework/Animation.h"
class Mario :
    public GameObject
{
    sf::Texture marioSpriteSheet;
    Animation Walk;
    Animation Swim;
    Animation Duck;
    Animation* currentAnimation;

public:
    Mario();
    void handleInput(float dt) override;
};

