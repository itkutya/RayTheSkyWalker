#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <memory>
#include <array>

#include "SFML/Graphics.hpp"

#define mapWidth 24
#define mapHeight 24
#define miniMapWidth 12
#define miniMapHeight 12
#define texWidth 64
#define texHeight 64

constexpr float PI = 3.141592653589793116f;

int worldMap[mapWidth][mapHeight] =
{
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,7,6,6,6,7,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,6,0,0,0,5,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

struct Sprite
{
    float x;
    float y;
    int texture;
    float uDiv  = 1.f;
    float vDiv  = 1.f;
    float vMove = 0.f;
};

#define numSprites 4

Sprite sprite[numSprites] =
{
  {22, 11, 3, 1.f, 1.f, 0.f},
  {21, 12, 2, 1.5f, 1.5f, 300.f},
  {20, 13, 4, 1.f, 2.f, -300.f},
  {21, 10, 1, 1.f, 1.f, 0.f}
};

double ZBuffer[1440];
int spriteOrder[numSprites];
double spriteDistance[numSprites];
void sortSprites(int* order, double* dist, int amount)
{
    std::vector<std::pair<double, int>> sprites(amount);
    for (int i = 0; i < amount; i++) {
        sprites[i].first = dist[i];
        sprites[i].second = order[i];
    }
    std::sort(sprites.begin(), sprites.end());

    for (int i = 0; i < amount; i++) {
        dist[i] = sprites[amount - i - 1].first;
        order[i] = sprites[amount - i - 1].second;
    }
};

int main()
{
    int screenWidth = 1440;
    int screenHeight = 800;

	sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Raycaster!");
    window.setFramerateLimit(60);
    //window.setVerticalSyncEnabled(true);
    //window.setMouseCursorVisible(false);

    sf::FloatRect visibleArea(0.f, 0.f, (float)screenWidth, (float)screenHeight);
    window.setView(sf::View(visibleArea));

    float posX = 20.f, posY = 12.f;
    float dirX = -1.f, dirY = 0.f;
    float planeX = 0.f, planeY = 0.7f;
    float angle = 0.f;

    sf::VertexArray line(sf::PrimitiveType::Lines, 2 * screenWidth);
    sf::VertexArray ground(sf::PrimitiveType::Quads, 4);
    ground[0].color = sf::Color(75, 75, 75, 255);
    ground[1].color = sf::Color(75, 75, 75, 255);
    ground[2].color = sf::Color::Black;
    ground[3].color = sf::Color::Black;
    sf::Clock deltaClock;
    sf::Color color;
    sf::Event event;

    //Out of range for bigger (window) stuff LOL, maybie sfml stuff? idk...
    std::vector<sf::VertexArray> entity;
    entity.reserve(numSprites);
    for (int i = 0; i < numSprites; ++i)
    {
        entity.push_back(sf::VertexArray(sf::PrimitiveType::Lines, 2 * screenWidth));
    }

    sf::Vector2i mousPos;
    sf::Font font;
    sf::Text text;

    if (!font.loadFromFile("res/sansation.ttf"))
        return -1;

    text.setFont(font);
    text.setPosition(screenWidth - 200.f, 50.f);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);

    int side;
    int stepX;
    int stepY;
    float perpWallDist;
    float sideDistX;
    float sideDistY;

    sf::Texture texture;
    if (!texture.loadFromFile("res/wolftextures.png"))
    {
        return -1;
    }
    texture.setRepeated(false);
    texture.setSmooth(true);
    texture.setSrgb(true);
    sf::RenderStates states;
    states.texture = &texture;

    while (window.isOpen())
    {
        float deltaTime = deltaClock.restart().asSeconds();
        text.setString(std::to_string((int)1 / deltaTime) + " FPS");

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
                window.close();
            if (event.type == sf::Event::Resized)
            {
                screenWidth = event.size.width;
                screenHeight = event.size.height;
                line.resize(screenWidth * 2);
                entity.clear();
                for (int i = 0; i < numSprites; ++i)
                {
                    entity.push_back(sf::VertexArray(sf::PrimitiveType::Lines, 2 * screenWidth));
                }
            }
        }

        float moveSpeed = deltaTime * 6.f;
        float sensitivity = deltaTime * 90.f;

        if (window.hasFocus())
        {
            mousPos = sf::Mouse::getPosition(window);

            float diff_X = (float)((screenWidth / 2) - (float)mousPos.x) / (float)screenWidth;
            float oldDirX = dirX;
            dirX = dirX * cos(sensitivity * diff_X) - dirY * sin(sensitivity * diff_X);
            dirY = oldDirX * sin(sensitivity * diff_X) + dirY * cos(sensitivity * diff_X);
            float oldPlaneX = planeX;
            planeX = planeX * cos(sensitivity * diff_X) - planeY * sin(sensitivity * diff_X);
            planeY = oldPlaneX * sin(sensitivity * diff_X) + planeY * cos(sensitivity * diff_X);

            float diff_Y = (float)((screenHeight / 2) - (float)mousPos.y) / (float)screenHeight;
            if (diff_Y > 0.f)
            {
                angle += 1.f * sensitivity * diff_Y;
            }
            else if (diff_Y < 0.f)
            {
                angle += 1.f * sensitivity * diff_Y;
            }
            angle = std::clamp(angle, 0.f, 2.f);

            sf::Mouse::setPosition(sf::Vector2i(window.getSize().x / 2, window.getSize().y / 2), window);

            //Fix here maybie
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            {
                if (worldMap[(int)(posX + dirX * moveSpeed)][(int)(posY)] == 0) posX += dirX * moveSpeed;
                if (worldMap[(int)(posX)][int(posY + dirY * moveSpeed)] == 0) posY += dirY * moveSpeed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            {
                if (worldMap[(int)(posX - dirX * moveSpeed)][(int)(posY)] == 0) posX -= dirX * moveSpeed;
                if (worldMap[(int)(posX)][(int)(posY - dirY * moveSpeed)] == 0) posY -= dirY * moveSpeed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                float oldDirX = dirX;
                if (worldMap[(int)(posX - (dirX * cos(PI / 2) - dirY * sin(PI / 2)) * moveSpeed)][(int)(posY)] == 0) posX -= (dirX * cos(PI / 2) - dirY * sin(PI / 2)) * moveSpeed;
                if (worldMap[(int)(posX)][(int)(posY - (oldDirX * sin(PI / 2) + dirY * cos(PI / 2)) * moveSpeed)] == 0) posY -= (oldDirX * sin(PI / 2) + dirY * cos(PI / 2)) * moveSpeed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                float oldDirX = dirX;
                if (worldMap[(int)(posX + (dirX * cos(PI / 2) - dirY * sin(PI / 2)) * moveSpeed)][(int)(posY)] == 0) posX += (dirX * cos(PI / 2) - dirY * sin(PI / 2)) * moveSpeed;
                if (worldMap[(int)(posX)][(int)(posY + (oldDirX * sin(PI / 2) + dirY * cos(PI / 2)) * moveSpeed)] == 0) posY += (oldDirX * sin(PI / 2) + dirY * cos(PI / 2)) * moveSpeed;
            }
        }

        ground[0].position = sf::Vector2f(0.f, (float)screenHeight * (angle / 2.f));
        ground[1].position = sf::Vector2f((float)screenWidth, (float)screenHeight * (angle / 2.f));
        ground[2].position = sf::Vector2f((float)screenWidth, (float)screenHeight);
        ground[3].position = sf::Vector2f(0.f, (float)screenHeight);

        for (int i = 0; i < screenWidth; ++i)
        {
            float cameraX = 2 * i / (float)screenWidth - 1;
            float rayDirX = dirX + planeX * cameraX;
            float rayDirY = dirY + planeY * cameraX;

            int mapX = int(posX);
            int mapY = int(posY);

            //3%
            float deltaDistX = (rayDirX == 0) ? 1e30f : std::abs(1 / rayDirX);
            float deltaDistY = (rayDirY == 0) ? 1e30f : std::abs(1 / rayDirY);

            int hit = 0;
            if (rayDirX < 0)
            {
                stepX = -1;
                sideDistX = (posX - mapX) * deltaDistX;
            }
            else
            {
                stepX = 1;
                sideDistX = (mapX + 1.0f - posX) * deltaDistX;
            }
            if (rayDirY < 0)
            {
                stepY = -1;
                sideDistY = (posY - mapY) * deltaDistY;
            }
            else
            {
                stepY = 1;
                sideDistY = (mapY + 1.0f - posY) * deltaDistY;
            }

            while (hit == 0)
            {
                if (sideDistX < sideDistY)
                {
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                }
                else
                {
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1;
                }
                if (worldMap[mapX][mapY] > 0) hit = 1;
            }

            if (side == 0) perpWallDist = (sideDistX - deltaDistX);
            else           perpWallDist = (sideDistY - deltaDistY);

            int lineHeight = (int)(screenHeight / perpWallDist);
            int drawStart = (int)(-lineHeight / 2 + screenHeight / 2 * angle);
            int drawEnd = (int)(lineHeight / 2 + screenHeight / 2 * angle);

            /*switch (worldMap[mapX][mapY])
            {
            case 1:  color = sf::Color::White;  break;
            case 2:  color = sf::Color::White;  break;
            case 3:  color = sf::Color::White;  break;
            case 4:  color = sf::Color::White;  break;
            default: color = sf::Color::White;  break;
            }*/
            color = sf::Color::White;

            if (side == 1) { color.r = sf::Uint8(color.r / 1.5f); color.g = sf::Uint8(color.g / 1.5f); color.b = sf::Uint8(color.b / 1.5f); }

            sf::Vertex* walls = &line[i * 2];
            walls[0].position = sf::Vector2f((float)i + 1, (float)drawStart);
            walls[1].position = sf::Vector2f((float)i + 1, (float)drawEnd);

            walls[0].color = color;
            walls[1].color = color;

            float wallX;
            if (side == 0) wallX = posY + perpWallDist * rayDirY;
            else           wallX = posX + perpWallDist * rayDirX;
            wallX -= floor((wallX));

            int texX = int(wallX * double(texWidth));
            if (side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
            if (side == 1 && rayDirY < 0) texX = texWidth - texX - 1;

            int mapNum = worldMap[mapX][mapY];
            walls[0].texCoords = sf::Vector2f(texX + (texWidth * mapNum + 0.5f), 0.f);
            walls[1].texCoords = sf::Vector2f(texX + (texWidth * mapNum + 0.5f), (float)texHeight);

            ZBuffer[i] = perpWallDist;
        }

        //TO MUCH CPU!!!
        for (unsigned int i = 0; i < entity.size(); ++i)
        {
            for (unsigned int j = 0; j < entity[i].getVertexCount(); ++j)
            {
                entity[i][j].position = sf::Vector2f();
            }
        }

        for (int i = 0; i < numSprites; ++i)
        {
            spriteOrder[i] = i;
            spriteDistance[i] = ((posX - sprite[i].x) * (posX - sprite[i].x) + (posY - sprite[i].y) * (posY - sprite[i].y));
        }
        sortSprites(spriteOrder, spriteDistance, numSprites);

        for (int a = 0; a < numSprites; ++a)
        {
            float spriteX = sprite[spriteOrder[a]].x - posX;
            float spriteY = sprite[spriteOrder[a]].y - posY;
            float invDet = 1.f / (planeX * dirY - dirX * planeY);
            float transformX = invDet * (dirY * spriteX - dirX * spriteY);
            float transformY = invDet * (-planeY * spriteX + planeX * spriteY);

            int spriteScreenX = int((screenWidth / 2) * (1 + transformX / transformY));

            int vMoveScreen = int(sprite[spriteOrder[a]].vMove / transformY);

            int spriteHeight = abs((int)(screenHeight / (transformY) / sprite[spriteOrder[a]].vDiv));
            int drawStartY = (int)(- spriteHeight / 2 + screenHeight / 2 * angle + vMoveScreen);
            int drawEndY = (int)(spriteHeight / 2 + screenHeight / 2 * angle + vMoveScreen);

            int spriteWidth = abs((int)(screenHeight / (transformY) / sprite[spriteOrder[a]].uDiv));
            int drawStartX = -spriteWidth / 2 + spriteScreenX;
            int drawEndX = spriteWidth / 2 + spriteScreenX;

            for (int stripe = drawStartX; stripe < drawEndX; ++stripe)
            {
                if (transformY > 0 && stripe > 0 && stripe < screenWidth && transformY < ZBuffer[stripe - 1])
                {
                    int texX = int((stripe - (-spriteWidth / 2 + spriteScreenX)) * texWidth / spriteWidth);
                    sf::Vertex* ent = &entity[a][stripe * 2];
                    ent[0].position = sf::Vector2f((float)(stripe), (float)drawStartY);
                    ent[1].position = sf::Vector2f((float)(stripe), (float)drawEndY);

                    ent[0].texCoords = sf::Vector2f((float)(texWidth * sprite[spriteOrder[a]].texture + texX + 0.5f), 0.f);
                    ent[1].texCoords = sf::Vector2f((float)(texWidth * sprite[spriteOrder[a]].texture + texX + 0.5f), texHeight);
                }
            }
        }

        window.clear(sf::Color::Cyan);
        visibleArea = sf::FloatRect(0.f, 0.f, (float)screenWidth, (float)screenHeight);
        window.setView(sf::View(visibleArea));
        window.draw(ground);
        window.draw(line, states);
        for (auto&e : entity)
        {
            window.draw(e, states);
        }
        window.setView(window.getDefaultView());
        window.draw(text);
        window.display();
    }

	return 0;
}

//Minimap
/*    
        sf::VertexArray m_vertices;
        m_vertices.setPrimitiveType(sf::PrimitiveType::Quads);
        m_vertices.resize(mapWidth * mapHeight * 4);
        sf::Color map_color;
        for (unsigned int i = 0; i < mapWidth; ++i)
            for (unsigned int j = 0; j < mapHeight; ++j)
            {
                if (worldMap[i][j] > 0)
                {
                    sf::Vertex* quad = &m_vertices[(i + j * mapWidth) * 4];
            
                    quad[0].position = sf::Vector2f((float)i * (float)miniMapWidth, (float)j * (float)miniMapHeight);
                    quad[1].position = sf::Vector2f(((float)i + 1.f) * (float)miniMapWidth, (float)j * (float)miniMapHeight);
                    quad[2].position = sf::Vector2f(((float)i + 1.f) * (float)miniMapWidth, ((float)j + 1.f) * (float)miniMapHeight);
                    quad[3].position = sf::Vector2f((float)i * (float)miniMapWidth, ((float)j + 1.f) * (float)miniMapHeight);

                    switch (worldMap[i][j])
                    {
                    case 1:  map_color = sf::Color::Red;    break;
                    case 2:  map_color = sf::Color::Magenta;break;
                    case 3:  map_color = sf::Color::White;  break;
                    case 4:  map_color = sf::Color::Blue;   break;
                    default: map_color = sf::Color::Yellow; break;
                    }
                    quad[0].color = map_color;
                    quad[1].color = map_color;
                    quad[2].color = map_color;
                    quad[3].color = map_color;
                }
            }

        sf::RectangleShape player;
        player.setSize(sf::Vector2f(5.f, 5.f));
        player.setFillColor(sf::Color::Blue);
        player.setOrigin(sf::Vector2f(player.getGlobalBounds().width / 2, player.getGlobalBounds().height / 2));
        sf::VertexArray direction(sf::PrimitiveType::Lines, 2);
        direction[0].color = sf::Color::Cyan;
        direction[1].color = sf::Color::Blue;
        sf::VertexArray plane(sf::PrimitiveType::Lines, 2);
        plane[0].color = sf::Color::White;
        plane[1].color = sf::Color::Magenta;

        player.setPosition(posX * miniMapWidth, posY * miniMapHeight);
        direction[0].position.x = (posX) * miniMapWidth;
        direction[0].position.y = (posY) * miniMapHeight;
        direction[1].position.x = (posX + dirX) * miniMapWidth;
        direction[1].position.y = (posY + dirY) * miniMapHeight;
        plane[0].position.x = (posX + dirX + planeX) * miniMapWidth;
        plane[0].position.y = (posY + dirY + planeY) * miniMapHeight;
        plane[1].position.x = (posX + dirX - planeX) * miniMapWidth;
        plane[1].position.y = (posY + dirY - planeY) * miniMapHeight;

        window.draw(m_vertices);
        window.draw(player);
        window.draw(direction);
        window.draw(plane);
*/