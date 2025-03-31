#include "LightTile.h"


LightTile::LightTile(float range)
{
    light.setRange(range);
    light.setFade(true);   // Enable fading effect for the light
    setSize(sf::Vector2f(50, 50));  // Size of the light tile
}

void LightTile::setPosition(sf::Vector2f& position)
{
    Tiles::setPosition(position);   // Set the position of the tile itself
    light.setPosition(position);    // Set the position of the light source
}

void LightTile::setPosition(float x, float y)
{

    Tiles::setPosition(sf::Vector2f(x,y));   // Set the position of the tile itself
    light.setPosition(sf::Vector2f(x, y));    // Set the position of the light source
}

candle::RadialLight& LightTile::getLight()
{
    return light;   // Access the light for rendering purposes
}

void LightTile::draw(sf::RenderWindow& window)
{
    window.draw(light);  // Draw the light in the SFML window
}
