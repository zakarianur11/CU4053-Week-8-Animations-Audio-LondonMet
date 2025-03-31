#include "TileManager.h"
#include "Collision.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include "Utilities.h"

TileManager::TileManager() : fog(candle::LightingArea::FOG, sf::Vector2f(0.f, 0.f), sf::Vector2f(SCREEN_WIDTH, SCREEN_HEIGHT))
{
    fog.setAreaColor(sf::Color::Black);
    fog.setAreaOpacity(0.9);

    filePath = "TilesData.txt";
    textureManager.loadTexturesFromDirectory("gfx/TileTextures");
    // Set up ImGui variables
    imguiWidth = SCREEN_WIDTH / 4;
    imguiHeight = SCREEN_HEIGHT;

    iseditingLights = false;
}

void TileManager::handleInput(float dt)
{
    sf::Vector2i pixelPos = sf::Vector2i(input->getMouseX(), input->getMouseY());
    sf::Vector2f worldPos = window->mapPixelToCoords(pixelPos, *view);

    if (!iseditingLights)
    {
        if (input->isLeftMouseDown()) {
            bool tileClicked = false;
            int clickedTileIndex = -1;

            // Check if any tile is clicked
            for (int i = 0; i < tiles.size(); ++i) {
                if (Collision::checkBoundingBox(tiles[i]->getCollisionBox(), sf::Vector2i(worldPos))) {
                    tileClicked = true;
                    clickedTileIndex = i;
                    break;
                }
            }

            if (tileClicked) {
                if (input->isKeyDown(sf::Keyboard::LControl) || input->isKeyDown(sf::Keyboard::RControl)) {
                    // Ctrl is held, toggle the selection state of the tile
                    if (selectedTileIndices.find(clickedTileIndex) == selectedTileIndices.end()) {
                        selectedTileIndices.insert(clickedTileIndex); // Add to selection
                        tiles[clickedTileIndex]->setEditing(true);
                    }
                    else {
                        selectedTileIndices.erase(clickedTileIndex); // Remove from selection
                        tiles[clickedTileIndex]->setEditing(false);
                    }
                }
                else {
                    // No Ctrl key, clear existing selections and select the new tile only
                    for (auto index : selectedTileIndices) {
                        tiles[index]->setEditing(false); // Set all currently selected tiles to not editing
                    }
                    selectedTileIndices.clear();
                    selectedTileIndices.insert(clickedTileIndex);
                    tiles[clickedTileIndex]->setEditing(true);
                }
            }
            else {
                // Clicked on empty space
                if (!(input->isKeyDown(sf::Keyboard::LControl) || input->isKeyDown(sf::Keyboard::RControl))) {
                    if (!selectedTileIndices.empty()) {
                        // Clear all current selections
                        for (auto index : selectedTileIndices) {
                            tiles[index]->setEditing(false); // Set all currently selected tiles to not editing
                        }
                        selectedTileIndices.clear();
                        // Set a flag or remember this state to know a clearing has just occurred
                        //recentlyCleared = true;
                    }
                    else {
                        // Create a new tile only if it was recently cleared and now clicking again on empty space
                        auto newTile = std::make_unique<Tiles>();
                        newTile->setPosition(worldPos.x, worldPos.y);
                        world->AddGameObject(*newTile);
                        tiles.push_back(std::move(newTile));
                        int newIndex = tiles.size() - 1; // Get the index of the newly added tile
                        selectedTileIndices.insert(newIndex); // Select the newly added tile
                        tiles[newIndex]->setEditing(true);
                        recentlyCleared = false; // Reset the flag
                    }
                }
                else {
                    // If Ctrl is held, only clear selection without adding a new tile
                    for (auto index : selectedTileIndices) {
                        tiles[index]->setEditing(false); // Set all currently selected tiles to not editing
                    }
                    selectedTileIndices.clear();
                    //recentlyCleared = false; // Ensure the flag is reset even if Ctrl was held
                }
            }
            input->setLeftMouse(Input::MouseState::UP); // Mark the mouse click as handled
        }

        // Handle input for the active tiles
        for (int index : selectedTileIndices) {
            tiles[index]->setInput(input);
            tiles[index]->handleInput(dt);
        }

        // Update the color of the tiles based on selection and tag
        for (int i = 0; i < tiles.size(); ++i) {
            if (selectedTileIndices.find(i) != selectedTileIndices.end()) {
                tiles[i]->setColor(sf::Color::Green); // Highlight selected tiles
            }
            else if (tiles[i]->getTag() == "Wall") {
                tiles[i]->setColor(sf::Color::Blue);
            }
            else {
                tiles[i]->setColor(sf::Color::Red);
            }
        }

        // Additional functionality like duplication and deletion...

        // Duplication
        if (input->isKeyDown(sf::Keyboard::LControl) || input->isKeyDown(sf::Keyboard::RControl)) {
            if (input->isKeyDown(sf::Keyboard::D)) {
                // Duplicate all selected tiles
                std::vector<std::unique_ptr<Tiles>> newTiles; // Temporarily store new tiles to avoid modifying the collection while iterating
                for (int index : selectedTileIndices) {
                    auto& tile = tiles[index];
                    auto duplicatedTile = std::make_unique<Tiles>();
                    duplicatedTile->setPosition(tile->getPosition());
                    duplicatedTile->setSize(tile->getSize());
                    duplicatedTile->setTag(tile->getTag());
                    duplicatedTile->setTexture(tile->getTexture()); // Ensure this method exists and works correctly
                    duplicatedTile->setTextureName(tile->getTextureName()); // Ensure this method exists and works correctly
                    duplicatedTile->setTrigger(tile->getTrigger());
                    duplicatedTile->setStatic(tile->getStatic());
                    duplicatedTile->setMassless(tile->getMassless());
                    newTiles.push_back(std::move(duplicatedTile));
                }

                // Add new tiles to the main collection and select them
                for (auto& newTile : newTiles) {
                    world->AddGameObject(*newTile);
                    int newIndex = tiles.size();
                    tiles.push_back(std::move(newTile));
                    //selectedTileIndices.insert(newIndex); // Select new tiles
                }

                input->setKeyUp(sf::Keyboard::D); // Prevent continuous duplication while the key is held down
            }
        }

        //Deletion
        if (input->isKeyDown(sf::Keyboard::Delete)) {
            // Sort selected indices in descending order to safely delete multiple tiles without invalidating indices
            std::vector<int> sortedIndices(selectedTileIndices.begin(), selectedTileIndices.end());
            std::sort(sortedIndices.rbegin(), sortedIndices.rend()); // Reverse sort

            for (int index : sortedIndices) {
                if (index >= 0 && index < tiles.size()) {
                    world->RemoveGameObject(*tiles[index]);
                    tiles.erase(tiles.begin() + index);
                }
            }
            selectedTileIndices.clear(); // Clear selection after deletion
            input->setKeyUp(sf::Keyboard::Delete); // Prevent continuous deletion while the key is held down
        }
   }
    else
    {
        //Adding new lights 
        if (input->isLeftMousePressed())
        {
            if (selectedLight == nullptr)  // No light is selected, so create a new one
            {
                auto l = std::make_unique<candle::RadialLight>();
                l->setRange(500);
                l->setFade(true);
                l->setBeamAngle(50);
                l->setColor(sf::Color::White);
                l->setPosition(sf::Vector2f(worldPos.x, worldPos.y));
                selectedLight = l.get();  // Keep the raw pointer of the newly created light
                lights.push_back(std::move(l));  // Add to lights vector
            }
            else  // A light is already selected, so deselect it (ready to create another light)
            {
                selectedLight = nullptr;
            }
        }

        if (selectedLight != nullptr)  // Edit the selected light if one exists
        {
            // Handle mouse wheel zoom inputs
            int wheelDelta = input->getMouseWheelDelta();
            if (wheelDelta != 0) {
                float zoomFactor = wheelDelta > 0 ? 0.9f : 1.1f;  // More significant zoom change per scroll
                float zoomAdjustment = pow(zoomFactor, abs(wheelDelta));  // Apply the factor power of scroll intensity
                CurrentLightRange *= zoomAdjustment;
            }

            // Beam ANGLE
            if (input->isKeyDown(sf::Keyboard::A))
            {
                CurrentLightAngle += (50 * dt);
            }
            if (input->isKeyDown(sf::Keyboard::D))
            {
                CurrentLightAngle -= (50 * dt);
            }

            // Beam Rotation
            if (input->isKeyDown(sf::Keyboard::W))
            {
                CurrentLightRotation += (50 * dt);
            }
            if (input->isKeyDown(sf::Keyboard::S))
            {
                CurrentLightRotation -= (50 * dt);
            }

            // Apply changes to the selected light
            selectedLight->setBeamAngle(CurrentLightAngle);
            selectedLight->setRange(CurrentLightRange);
            selectedLight->setRotation(CurrentLightRotation);
            Output("Current Light Angle: %f\n", CurrentLightAngle);

        }

        //Adding New lighting Area 
        if (input->isRightMousePressed())
        {
            if (selectedArea == nullptr)  // No area is selected, so create a new one
            {
                auto area = std::make_unique<candle::LightingArea>(candle::LightingArea::AMBIENT,
                    sf::Vector2f(worldPos.x, worldPos.y),
                    sf::Vector2f(100.f, 100.f));  // Initial size
                area->setAreaColor(sf::Color::Red);
                area->setAreaOpacity(0.5f);
                selectedArea = area.get();  // Keep a raw pointer to the new area
                areas.push_back(std::move(area));  // Add the area to the areas vector
            }
            else  // Deselect the current area, allowing a new one to be created next time
            {
                selectedArea = nullptr;
            }
        }

        // Edit the selected lighting area if one exists
        if (selectedArea != nullptr)
        {
            // Adjust position with WASD keys
            if (input->isKeyDown(sf::Keyboard::W))
            {
                selectedArea->setPosition(selectedArea->getPosition() + sf::Vector2f(0.f, -100.f * dt));
            }
            if (input->isKeyDown(sf::Keyboard::S))
            {
                selectedArea->setPosition(selectedArea->getPosition() + sf::Vector2f(0.f, 100.f * dt));
            }
            if (input->isKeyDown(sf::Keyboard::A))
            {
                selectedArea->setPosition(selectedArea->getPosition() + sf::Vector2f(-100.f * dt, 0.f));
            }
            if (input->isKeyDown(sf::Keyboard::D))
            {
                selectedArea->setPosition(selectedArea->getPosition() + sf::Vector2f(100.f * dt, 0.f));
            }

            // Adjust size with IJKL keys
            sf::Vector2f currentSize = selectedArea->getScale();
            if (input->isKeyDown(sf::Keyboard::I))
            {
                currentSize.y -= 5.f * dt;  // Decrease height
            }
            if (input->isKeyDown(sf::Keyboard::K))
            {
                currentSize.y += 5.f * dt;  // Increase height
            }
            if (input->isKeyDown(sf::Keyboard::J))
            {
                currentSize.x -= 5.f * dt;  // Decrease width
            }
            if (input->isKeyDown(sf::Keyboard::L))
            {
                currentSize.x += 5.f * dt;  // Increase width
            }
            selectedArea->setScale(currentSize);
        }
    }
}


void TileManager::update(float dt)
{
    for (auto& tilePtr : tiles) {
        if (tilePtr) { // Check if the pointer is not null
            tilePtr->update(dt); // Dereference the pointer to get the Tiles object
        }
    }
}

void TileManager::render(bool editMode) {
    for (auto& tilePtr : tiles) {
        if (tilePtr) { // Check if the pointer is not null
            if (editMode) {
                sf::RectangleShape rect = tilePtr->getDebugCollisionBox();
                rect.setOutlineThickness(5);
                int tileIndex = &tilePtr - &tiles[0]; // Get index of the tile

                // Highlight selected tiles
                if (selectedTileIndices.find(tileIndex) != selectedTileIndices.end()) {
                    rect.setOutlineColor(sf::Color::Green);
                } else {
                    rect.setOutlineColor(sf::Color::Red);
                }
                
                window->draw(rect);
            }
            if(!tilePtr->getTexture()==NULL) window->draw(*tilePtr); // Draw the tile
        }
    }


}

void TileManager::saveTiles(const std::vector<std::unique_ptr<Tiles>>& tiles, const std::string& filePath)
{
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cout << "Failed to open file for saving tiles." << std::endl;
        return;
    }

    for (const auto& tile : tiles) {
        file << tile->getTag() << ","
            << tile->getPosition().x << ","
            << tile->getPosition().y << ","
            << tile->getSize().x << ","
            << tile->getSize().y << ","
            << tile->getTrigger() << ","
            << tile->getStatic() << ","
            << tile->getMassless() << ","
            << tile->getTile() << ","
            << tile->getTextureName() << "\n";  // Ensure the texture name is always written, even if it's empty
    }
}

bool TileManager::loadTiles()
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cout << "Failed to open file for loading tiles." << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream linestream(line);
        std::string segment;
        std::vector<std::string> seglist;

        while (std::getline(linestream, segment, ',')) {
            seglist.push_back(segment);
        }

        if (seglist.size() >= 9) {
            auto newTile = std::make_unique<Tiles>();
            newTile->setTag(seglist[0]);
            newTile->setPosition(sf::Vector2f(std::stof(seglist[1]), std::stof(seglist[2])));
            newTile->setSize(sf::Vector2f(std::stof(seglist[3]), std::stof(seglist[4])));
            newTile->setTrigger(std::stoi(seglist[5]));
            newTile->setStatic(std::stoi(seglist[6]));
            newTile->setMassless(std::stoi(seglist[7]));
            newTile->setTile(std::stoi(seglist[8]));

            // Check if a texture name exists and is valid
            if (seglist.size() > 9 && !seglist[9].empty()) {
                newTile->setTextureName(seglist[9]);
                sf::Texture* texture = textureManager.getTexture(seglist[9]);
                if (texture) {
                    newTile->setTexture(texture);
                }
            }

            world->AddGameObject(*newTile);
            tiles.push_back(std::move(newTile));
        }
    }

    return true;
}


std::vector<std::unique_ptr<Tiles>>& TileManager::getTiles() {
    return tiles;
}

void TileManager::RemoveCollectable()
{
    auto newEnd = std::remove_if(tiles.begin(), tiles.end(),
        [this](const std::unique_ptr<Tiles>& tilePtr) -> bool
        {
            if (tilePtr->CollisionWithTag("Player") && tilePtr->getTag()== "Collectable")
            {
                world->RemoveGameObject(*tilePtr);
                return true; // Mark for removal
            }
            return false; // Keep in the vector
        });

    tiles.erase(newEnd, tiles.end());
}

void TileManager::DrawImGui() {
    ImVec2 imguiSize(imguiWidth, imguiHeight);
    ImVec2 imguiPos(SCREEN_WIDTH - imguiWidth, 0); // Positioned on the right-hand side

    // Set the window size
    ImGui::SetNextWindowSize(imguiSize);

    // Set the window position
    ImGui::SetNextWindowPos(imguiPos);

    // Window flags
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoMove;          // The window will not be movable
    window_flags |= ImGuiWindowFlags_NoResize;        // Disable resizing
    window_flags |= ImGuiWindowFlags_NoCollapse;      // Disable collapsing
    //window_flags |= ImGuiWindowFlags_NoTitleBar;      // Disable the title bar
    //window_flags |= ImGuiWindowFlags_NoScrollbar;     // Disable the scrollbar

    if (ImGui::Begin("Tile Editor", nullptr, window_flags)) 
    {
        if (ImGui::CollapsingHeader("Help"))
        {
            ImGui::Text("Left Click: Place Tile");
            ImGui::Text("Right Click and Drag: Move Camera");
            ImGui::Text("Delete: Delete Tile");
            ImGui::Text("Ctrl+D: Duplicate Tile");
            ImGui::Text("Tab: Save and Exit");
        }

        if (ImGui::CollapsingHeader("Best Checkbox Combinations"))
        {
            if (ImGui::CollapsingHeader("Checkpoint:"))
            {
                ImGui::Text("Should be set as a Trigger, Static, and Tile.");
                if (ImGui::CollapsingHeader("Explanation:"))
                    ImGui::Text("Checkpoints need to be interacted with but should not\nimpede player movement or be affected by physics.");
            }


            if (ImGui::CollapsingHeader("Coin:"))
            {
                ImGui::Text("Should be set as a Trigger, Massless, and Tile.");
                if (ImGui::CollapsingHeader("Explanation:"))
                    ImGui::Text("Coins should be collectable without affecting the physics\nof the player or game environment.");
            }


            if (ImGui::CollapsingHeader("Moving Platform:"))
            {

                ImGui::Text("Should not be a Trigger or Massless, but Static if not moving vertically.");
                if (ImGui::CollapsingHeader("Explanation:"))
                    ImGui::Text("Platforms that carry the player or objects should interact with \nphysics correctly and not trigger events.");
            }
        }

        if (ImGui::BeginTabBar("Tile Editor Tabs")) {
            if (ImGui::BeginTabItem("Tiles")) {
                // Tiles List inside a ListBox with collapsible groups
                if (ImGui::BeginListBox("Tile List", ImVec2(200, 300))) {


                    // Step 1: Create a map to store tiles by their tags
                    std::map<std::string, std::vector<int>> tilesByTag;

                    for (int i = 0; i < tiles.size(); i++) {
                        std::string tag =  tiles[i]->getTag();
                        tilesByTag[tag].push_back(i);
                    }

                    // Step 2: Iterate over each tag group and create a collapsible section
                    for (auto& group : tilesByTag) {
                        const std::string& tag = group.first;
                        const std::vector<int>& tileIndices = group.second;

                        if (tag.empty()) {
                            if (ImGui::TreeNodeEx("##emptyTag", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) {
                                // List the tiles under the collapsible section
                                for (int idx : tileIndices) {
                                    std::string item_label = "Tile " + std::to_string(idx) + "##" + std::to_string(idx);

                                    bool isSelected = selectedTileIndices.find(idx) != selectedTileIndices.end();
                                    if (ImGui::Selectable(item_label.c_str(), isSelected)) {
                                        if (ImGui::GetIO().KeyCtrl) {
                                            // Toggle selection with Ctrl pressed
                                            if (isSelected) {
                                                selectedTileIndices.erase(idx);
                                            }
                                            else {
                                                selectedTileIndices.insert(idx);
                                            }
                                        }
                                        else {
                                            // Single selection
                                            selectedTileIndices.clear();
                                            selectedTileIndices.insert(idx);
                                        }
                                    }
                                }
                                ImGui::TreePop();  // Close the group after listing its tiles
                            }
                        }
                        else {
                            if (ImGui::TreeNodeEx(tag.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) {
                                // List the tiles under the collapsible section
                                for (int idx : tileIndices) {
                                    std::string item_label = "Tile " + std::to_string(idx) + "##" + std::to_string(idx);

                                    bool isSelected = selectedTileIndices.find(idx) != selectedTileIndices.end();
                                    if (ImGui::Selectable(item_label.c_str(), isSelected)) {
                                        if (ImGui::GetIO().KeyCtrl) {
                                            // Toggle selection with Ctrl pressed
                                            if (isSelected) {
                                                selectedTileIndices.erase(idx);
                                            }
                                            else {
                                                selectedTileIndices.insert(idx);
                                            }
                                        }
                                        else {
                                            // Single selection
                                            selectedTileIndices.clear();
                                            selectedTileIndices.insert(idx);
                                        }
                                    }
                                }
                                ImGui::TreePop();  // Close the group after listing its tiles
                            }
                        }
                    }

                    ImGui::EndListBox();
                }

                if (!selectedTileIndices.empty()) {

                    // Buttons for setting properties to common types
                    if (ImGui::Button("Convert to Collectable")) {
                        for (int idx : selectedTileIndices) {
                            auto& tile = *tiles[idx];
                            tile.setMassless(true);
                            tile.setTrigger(true);
                            tile.setTile(true);
                            tile.setStatic(false);
                            tile.setTag("Collectable");
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Use these settings to convert the selected tile(s) to a Collectable.");
                    }

                    if (ImGui::Button("Convert to Platform")) {
                        for (int idx : selectedTileIndices) {
                            auto& tile = *tiles[idx];
                            tile.setStatic(true);
                            tile.setTile(true);
                            tile.setTrigger(false);
                            tile.setMassless(false);
                            tile.setTag("Platform");
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Use these settings to convert the selected tile(s) to a Platform.");
                    }

                    if (ImGui::Button("Convert to Checkpoint")) {
                        for (int idx : selectedTileIndices) {
                            auto& tile = *tiles[idx];
                            tile.setStatic(true);
                            tile.setTrigger(true);
                            tile.setTile(true);
                            tile.setMassless(false);
                            tile.setTag("Checkpoint");
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Use these settings to convert the selected tile(s) to a Checkpoint.");
                    }

                    ImGui::Text("Selected Tiles: %d", (int)selectedTileIndices.size());
                    displayTilePositions();  // Edit positions
                    displayTileScales();     // Edit scales

                    // Display properties if tiles have the same tag or only one is selected
                    if (selectedTileIndices.size() == 1 || allTilesHaveSameTag()) {
                        auto& firstTile = *tiles[*selectedTileIndices.begin()];
                        displayTileProperties(firstTile);
                    }

                    displayTextureSelection(textureManager);
                }
                
                if (ImGui::Button("Add New Tile")) {
                    addNewTile();
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete Selected Tiles")) {
                    deleteSelectedTiles();
                }


                if (ImGui::Button("Save")) {
                    saveTiles(tiles, filePath);
                }

                ImGui::EndTabItem();

                
            }

            if (ImGui::BeginTabItem("Lights")) {
                iseditingLights = true;

                if (ImGui::BeginListBox("Lights List", ImVec2(200, 300))) {
                    for (size_t i = 0; i < lights.size(); i++) {
                        std::string item_label = "Light " + std::to_string(i) + "##" + std::to_string(i);

                        bool isSelected = (selectedLight == lights[i].get());
                        if (ImGui::Selectable(item_label.c_str(), isSelected)) {
                            if (ImGui::GetIO().KeyCtrl) {
                                // Toggle selection with Ctrl
                                if (isSelected) {
                                    selectedLight = nullptr;
                                }
                                else {
                                    selectedLight = lights[i].get();
                                }
                            }
                            else {
                                // Single selection
                                selectedLight = lights[i].get();
                            }
                        }
                    }
 

                    ImGui::EndListBox();
                }
                // Only show the color selection if a light is selected
                if (selectedLight) {
                    static const char* colorNames[] = { "Black", "White", "Red", "Green", "Blue", "Yellow", "Magenta", "Cyan", "Transparent" };
                    static sf::Color colorValues[] = { sf::Color::Black, sf::Color::White, sf::Color::Red, sf::Color::Green,
                                                       sf::Color::Blue, sf::Color::Yellow, sf::Color::Magenta, sf::Color::Cyan, sf::Color::Transparent };

                    static int currentColorIndex = 0;

                    // Find current color index
                    for (int i = 0; i < IM_ARRAYSIZE(colorValues); i++) {
                        if (selectedLight->getColor() == colorValues[i]) {
                            currentColorIndex = i;
                            break;
                        }
                    }

                    if (ImGui::BeginCombo("Light Color", colorNames[currentColorIndex])) {
                        for (int i = 0; i < IM_ARRAYSIZE(colorNames); i++) {
                            bool isSelected = (currentColorIndex == i);
                            if (ImGui::Selectable(colorNames[i], isSelected)) {
                                currentColorIndex = i;
                                selectedLight->setColor(colorValues[i]);  // Update light color

                                if (isSelected) {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }
                }
                if (selectedLight) {
                    sf::Vector2f position = selectedLight->getPosition();
                    float range = selectedLight->getRange();
                    float angle = selectedLight->getBeamAngle();
                    float rotation = selectedLight->getRotation();

                    // Position DragFloat2
                    if (ImGui::DragFloat2("Position", &position.x, 0.5f, 0.0f, 10000.0f, "%.3f")) {
                        selectedLight->setPosition(position);
                    }

                    // Range DragFloat
                    if (ImGui::DragFloat("Range", &range, 1.0f, 10.0f, 1000.0f, "%.1f")) {
                        selectedLight->setRange(range);
                    }

                    // Beam Angle DragFloat
                    if (ImGui::DragFloat("Beam Angle", &angle, 1.0f, 0.0f, 180.0f, "%.1f")) {
                        selectedLight->setBeamAngle(angle);
                    }

                    // Rotation DragFloat
                    if (ImGui::DragFloat("Rotation", &rotation, 1.0f, -360.0f, 360.0f, "%.1f")) {
                        selectedLight->setRotation(rotation);
                    }
                }
                ImGui::Text("Work in Progress.....");
                ImGui::EndTabItem();
            }
            else
                iseditingLights = false;
            ImGui::EndTabBar();
        }

        

        ImGui::End();
    }
}


void TileManager::displayTextureSelection(TextureManager& textureManager) {
    if (selectedTileIndices.empty()) return;

    // Assume first selected tile's texture as the default for simplicity
    std::string currentTextureName = tiles[*selectedTileIndices.begin()]->getTextureName();
    const std::vector<std::string>& textureNames = textureManager.getTextureNames();

    // Find the current index based on the texture name
    int current_item = 0; // Default to the first texture if not found
    for (int i = 0; i < textureNames.size(); i++) {
        if (textureNames[i] == currentTextureName) {
            current_item = i;
            break;
        }
    }

    // Start the ImGui combo box
    if (ImGui::BeginCombo("Texture", currentTextureName.c_str())) {
        for (int n = 0; n < textureNames.size(); n++) {
            bool is_selected = (current_item == n);
            if (ImGui::Selectable(textureNames[n].c_str(), is_selected)) {
                // Set the new current item
                current_item = n;
                // Update the texture on all selected tiles
                sf::Texture* selectedTexture = textureManager.getTexture(textureNames[n]);
                for (auto idx : selectedTileIndices) 
                {
                    tiles[idx]->setTexture(selectedTexture,true);
                    tiles[idx]->setTextureName(textureNames[n]);  // Save the texture name
                }
                
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();  // Automatically scroll to the selected item
            }
        }
        ImGui::EndCombo();
    }
}



void TileManager::displayTilePositions() {
    if (selectedTileIndices.empty()) return;

    // Compute an average position to start with for simplicity
    sf::Vector2f averagePos(0, 0);
    for (int idx : selectedTileIndices) {
        averagePos += tiles[idx]->getPosition();
    }
    averagePos.x /= selectedTileIndices.size();
    averagePos.y /= selectedTileIndices.size();

    sf::Vector2f newPos = averagePos;
    if (ImGui::DragFloat2("Position", &newPos.x, 0.5f, 0, 0, "%.3f")) {
        sf::Vector2f deltaPos = newPos - averagePos;
        for (int idx : selectedTileIndices) {
            sf::Vector2f currentPos = tiles[idx]->getPosition();
            tiles[idx]->setPosition(currentPos + deltaPos);
        }
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Drag or double-click to edit.");
    }
}

void TileManager::displayTileScales() {
    if (selectedTileIndices.empty()) return;

    // Compute an average scale to start with for simplicity
    sf::Vector2f averageScale(0, 0);
    for (int idx : selectedTileIndices) {
        averageScale += tiles[idx]->getSize();
    }
    averageScale.x /= selectedTileIndices.size();
    averageScale.y /= selectedTileIndices.size();

    sf::Vector2f newScale = averageScale;
    if (ImGui::DragFloat2("Scale", &newScale.x, 0.1f, 0.01f, 1000.0f, "%.3f")) {
        sf::Vector2f deltaScale = newScale - averageScale;
        for (int idx : selectedTileIndices) {
            sf::Vector2f currentScale = tiles[idx]->getSize();
            tiles[idx]->setSize(currentScale + deltaScale);
        }
    }
    if (ImGui::IsItemHovered())
    {
		ImGui::SetTooltip("Drag or double-click to edit.");
	}
}


void TileManager::displayTileProperties(Tiles& tile) 
{
    if (selectedTileIndices.empty() || !allTilesHaveSameTag()) return;

    auto& firstTile = *tiles[*selectedTileIndices.begin()];

    char buffer[256];
    strcpy_s(buffer, firstTile.getTag().c_str());
    if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
        for (int idx : selectedTileIndices) {
            tiles[idx]->setTag(std::string(buffer));
        }
    }
    if (ImGui::IsItemActive())
    {
        inputTextActive = true;
    }
    else
    {
        inputTextActive = false;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Enter Tag, this can be used during collision detection");
    }

    bool istrigger = firstTile.getTrigger();
    bool isstatic = firstTile.getStatic();
    bool ismassless = firstTile.getMassless();
    bool istile = firstTile.getTile();

    displayCheckBox("Trigger", istrigger);
    displayCheckBox("Static", isstatic);
    displayCheckBox("Massless", ismassless);
    displayCheckBox("Tile", istile);
}


bool TileManager::allTilesHaveSameTag() {
    if (selectedTileIndices.size() < 2) return true;
    std::string firstTag = tiles[*selectedTileIndices.begin()]->getTag();
    for (auto idx : selectedTileIndices) {
        if (tiles[idx]->getTag() != firstTag) return false;
    }
    return true;
}

void TileManager::displayCheckBox(const char* label, bool& value) {
    bool currentValue = value;
    if (ImGui::Checkbox(label, &currentValue)) {
        for (int idx : selectedTileIndices) {
            if (strcmp(label, "Trigger") == 0) {
                tiles[idx]->setTrigger(currentValue);
            }
            else if (strcmp(label, "Static") == 0) {
                tiles[idx]->setStatic(currentValue);
            }
            else if (strcmp(label, "Massless") == 0) {
                tiles[idx]->setMassless(currentValue);
            }
            else if (strcmp(label, "Tile") == 0) {
                tiles[idx]->setTile(currentValue);
            }
        }
    }
    if (ImGui::IsItemHovered()) {
        if (strcmp(label, "Trigger") == 0) {
            ImGui::SetTooltip("Mark the tile as a trigger. Triggers do not impede player movement and are often used for items like checkpoints and collectables that execute actions on contact.");
        }
        else if (strcmp(label, "Static") == 0) {
            ImGui::SetTooltip("Set the tile to be static. Static tiles do not move and cannot be affected by physics, suitable for immovable objects like walls.");
        }
        else if (strcmp(label, "Massless") == 0) {
            ImGui::SetTooltip("Make the tile massless. Massless tiles are not affected by gravitational forces and are typically used for items that should not fall or weigh down. Collision is still detected");
        }
        else if (strcmp(label, "Tile") == 0) {
            ImGui::SetTooltip("Mark the object as a tile. Used for general tile properties in the game's level design. Tiles do not collide with other tiles.");
        }
    }
}

void TileManager::addNewTile() {
    auto newTile = std::make_unique<Tiles>();
    newTile->setPosition(0, 0);  // Default position
    world->AddGameObject(*newTile);
    tiles.push_back(std::move(newTile));
    selectedTileIndices.clear();
    selectedTileIndices.insert(tiles.size() - 1);
}

void TileManager::addLight(sf::Vector2f position)
{
    auto light = std::make_unique<candle::RadialLight>();
    light->setRange(CurrentLightRange);
    light->setFade(true);
    light->setBeamAngle(CurrentLightAngle);
    light->setColor(sf::Color::White);
    light->setPosition(position);
    selectedLight = light.get();
    lights.push_back(std::move(light));
}

void TileManager::removeSelectedLight() {
    if (selectedLight) {
        lights.erase(std::remove_if(lights.begin(), lights.end(),
            [&](const std::unique_ptr<candle::RadialLight>& l) {
                return l.get() == selectedLight;
            }), lights.end());
        selectedLight = nullptr;
    }
}

void TileManager::deleteSelectedTiles() {
    std::vector<int> sortedIndices(selectedTileIndices.begin(), selectedTileIndices.end());
    std::sort(sortedIndices.rbegin(), sortedIndices.rend());
    for (int idx : sortedIndices) {
        world->RemoveGameObject(*tiles[idx]);
        tiles.erase(tiles.begin() + idx);
    }
    selectedTileIndices.clear();
}


void TileManager::updateLighting() {
    // Clear and redraw fog
    fog.clear();
    for (auto& lightPtr : lights) {
        fog.draw(*lightPtr);
    }
    fog.display();

    //// Update each lighting area
    //for (auto& areaPtr : areas) {
    //    areaPtr->clear();
    //    for (auto& lightPtr : lights) {
    //        areaPtr->draw(*lightPtr);
    //    }
    //    areaPtr->display();
    //}

}

void TileManager::renderLighting()
{
    if (iseditingLights)
    {
        for (auto& lightPtr : lights)
        {
            lightPtr->castLight(edges.begin(), edges.end());
        }

        for (auto& lightPtr : lights)
        {
            window->draw(*lightPtr);
        }

        //for (auto& areaPtr : areas)
        //{
        //    window->draw(*areaPtr);
        //}

        window->draw(fog);
    }
}



