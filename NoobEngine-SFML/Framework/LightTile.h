#pragma once
#include "Tiles.h"
#include "RadialLight.hpp"

class LightTile : public Tiles {

private:
    candle::RadialLight light;  // Light source for the tile

public:
    LightTile(float range = 1000.0f);

    void setPosition(sf::Vector2f& position);
    void setPosition(float x, float y);

    candle::RadialLight& getLight();

    void setRange(float range);

    void setColour(sf::Color c);

    void draw(sf::RenderWindow& window);
};
