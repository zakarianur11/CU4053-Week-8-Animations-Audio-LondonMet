#pragma once
#include "GameObject.h"
#include "World.h"
#include "Tiles.h"
#include "TextureManager.h"
#include "LightTile.h"
#include <fstream>
#include <vector>
#include <string>
#include <sstream> // This is required for std::stringstream
#include <set>     // For selecting multiple tiles
#include "candle.hpp"

class TileManager : public GameObject
{
    std::set<int> selectedTileIndices; // Set to keep track of selected tile indices
    bool recentlyCleared = false; // Used to prevent multiple tiles from being selected at once
    int activeTileIndex = -1; // -1 indicates no tile is actively being edited
    bool tilesLoaded = false;

    std::vector<std::unique_ptr<Tiles>> tiles;
    
    //Lighting Test 
    // create a light source
    std::vector<std::unique_ptr<candle::RadialLight>> lights;
    candle::RadialLight* selectedLight = nullptr;

    std::vector<std::unique_ptr<candle::LightingArea>> areas;
    candle::LightingArea* selectedArea = nullptr;


    //This is like a canvas, without this I cannot draw any lights onto the screen. 
    //I dont know if I want to keep adding additional Lighting area, casue the same effect can
    //achieved by using different light colors. Maybe not 
    candle::LightingArea fog;

    //Creating Edgevector, this is for shadows, but I am not planning on using this 
    //Maybe in the future when i want objects to caste shadows thats when I will use this 
    //Also you need this to actually caste light.
    //Its empty for now. 
    candle::EdgeVector edges;

    //Testing Values, this can be changed via imgui editor. 
    float CurrentLightRange = 500.f;
    float CurrentLightAngle = 50.f;
    float CurrentLightRotation = 50.f;


    //Only edit lights if handleInput is disabled 
    bool iseditingLights;

    TextureManager textureManager;

    std::string filePath; // File to store tile data

    World* world;
    sf::View* view;

    bool showDebugCollisionBox;

    //ImGui variables
    float imguiWidth;
    float imguiHeight;
    bool inputTextActive;

public:
    TileManager();

    //Lighting Test 
    void addLight(sf::Vector2f position);
    void removeSelectedLight();
    void updateLighting();
    void renderLighting();

    void update(float dt) override;
    void handleInput(float dt) override;
    void render(bool editMode);

    void saveTiles(const std::vector<std::unique_ptr<Tiles>>& tiles, const std::string& filePath);
    bool loadTiles();

    std::vector<std::unique_ptr<Tiles>>& getTiles();

    void setWorld(World* world) { this->world = world; }
    void setView(sf::View* view) { this->view = view; }

    std::string getFilePath() { return filePath; }

    void RemoveCollectable();

    void ShowDebugCollisionBox(bool b) { showDebugCollisionBox = b; }

    void DrawImGui();

    void displayTilePositions();
    void displayTileScales();

    bool isInputTextActive() { return inputTextActive; }

    void displayTextureSelection(TextureManager& textureManager);

    bool allTilesHaveSameTag();
    void displayTileProperties(Tiles& tile);
    void displayCheckBox(const char* label, bool& value);
    void addNewTile();


    void deleteSelectedTiles();
};