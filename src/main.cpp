#include <SFML/Graphics.hpp>
#include "player.h"
#include "ZombieArena.h"
#include <iostream>
#include "TextureHolder.h"
#include "bullet.h"
using namespace sf;

int main() 
{
	TextureHolder holder;

	enum class State
	{
		PAUSED, LEVELING_UP, GAME_OVER, PLAYING
	};

	//start with game_over state
	State state = State::GAME_OVER;

	//resolution
	Vector2f resolution;
	resolution.x = VideoMode::getDesktopMode().width;
	resolution.y = VideoMode::getDesktopMode().height;
	RenderWindow window(VideoMode(resolution.x, resolution.y), "Zombie Arena", Style::Fullscreen);

	//sfml view 
	View mainView(FloatRect(0, 0, resolution.x, resolution.y));

	//time stuff
	Clock clock;
	Time gameTimeTotal;

	//mousePos
	Vector2f mouseWorldPosition;
	Vector2i mouseScreenPosition;

	//game stuff
	Player player;
	IntRect arena;

	//create bg
	VertexArray background;

	//Load the texture for bg va
	Texture textureBackground = TextureHolder::GetTexture("resources/graphics/background_sheet.png");

	int numZombies = 0;
	int numZombiesAlive = 0;
	Zombie* zombies = nullptr;

	Bullet bullets[100];
	int currentBullet = 0;
	int bulletsSpare = 24;
	int bulletsInClip = 6;
	int clipSize = 6;
	float fireRate = 1;
	Time lastPressed;

	//Hide the mouse pointer and replace it with a crosshair
	window.setMouseCursorVisible(false);
	Sprite spriteCrosshair;
	Texture textureCrosshair = TextureHolder::GetTexture("resources/graphics/crosshair.png");
	spriteCrosshair.setTexture(textureCrosshair);
	spriteCrosshair.setOrigin(25, 25);

	//game loop
	while (window.isOpen())
	{
		/*
		************
		HANDLE INPUT
		************
		*/

		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::KeyPressed)
			{
				if (state == State::PLAYING)
				{
					if (event.key.code == Keyboard::Return)
					{
						state = State::PAUSED;
					}
					else if (event.key.code == Keyboard::R)
					{
						if (bulletsSpare >= clipSize)
						{
							//Plenty of bullets. Reload.
							bulletsInClip = clipSize;
							bulletsSpare -= clipSize;
						}
						else if (bulletsSpare > 0)
						{
							//Only few bullets left
							bulletsInClip = bulletsSpare;
							bulletsSpare = 0;
						}
						else
						{
							//More here later - reload failed
						}
					}
				}
				else if (event.key.code == Keyboard::Return && state == State::PAUSED)
				{
					state = State::PLAYING;
					clock.restart();
				}
				else if (event.key.code == Keyboard::Return && state == State::GAME_OVER)
				{
					state = State::LEVELING_UP;
				}
				else if (event.key.code == Keyboard::Escape)
				{
					window.close();
				}
				else if (state == State::LEVELING_UP)
				{
					if (event.key.code == Keyboard::Num1)
					{
						state = State::PLAYING;
					}
					else if (event.key.code == Keyboard::Num2)
					{
						state = State::PLAYING;
					}
					else if (event.key.code == Keyboard::Num3)
					{
						state = State::PLAYING;
					}
					else if (event.key.code == Keyboard::Num4)
					{
						state = State::PLAYING;
					}
					else if (event.key.code == Keyboard::Num5)
					{
						state = State::PLAYING;
					}
					else if (event.key.code == Keyboard::Num6)
					{
						state = State::PLAYING;
					}
					if (state == State::PLAYING)
					{
						//prep level
						arena.width = 500;
						arena.height = 500;
						arena.left = 0;
						arena.top = 0;

						int tileSize = createBackground(background, arena);

						//spawn player in the middle
						player.spawn(arena, resolution, tileSize);

						numZombies = 10;
						delete[] zombies;
						zombies = createHorde(numZombies, arena);
						numZombiesAlive = numZombies;
						clock.restart();
					}
				}// end leveling up
			}// end is key pressed
		}// end of polling

		if (state == State::PLAYING)
		{
			if (Keyboard::isKeyPressed(Keyboard::W))
			{
				player.moveUp();
			}
			else
			{
				player.stopUp();
			}
			if (Keyboard::isKeyPressed(Keyboard::S))
			{
				player.moveDown();
			}
			else
			{
				player.stopDown();
			}
			if (Keyboard::isKeyPressed(Keyboard::A))
			{
				player.moveLeft();
			}
			else
			{
				player.stopLeft();
			}
			if (Keyboard::isKeyPressed(Keyboard::D))
			{
				player.moveRight();
			}
			else
			{
				player.stopRight();
			}
			if (Keyboard::isKeyPressed(Keyboard::P))
			{
				if(gameTimeTotal.asMilliseconds() - lastPressed.asMilliseconds() > 1000 / fireRate && bulletsInClip > 0) 
				{
					bullets[currentBullet].shoot(player.getCenter().x, player.getCenter().y, mouseWorldPosition.x, mouseWorldPosition.y);
					currentBullet++;
					if(currentBullet > 99) 
					{
						currentBullet = 0;
					}
					lastPressed = gameTimeTotal;
					bulletsInClip--;
				}//end fire bullet
			}
		}// end is playing input
		

		/*
		****************
		UPDATE THE FRAME
		****************
		*/

		if (state == State::PLAYING)
		{
			Time dt = clock.restart();
			gameTimeTotal += dt;
			float dtAsSeconds = dt.asSeconds();

			//find mouse
			mouseScreenPosition = Mouse::getPosition(window);

			//mouse screen -> world 
			mouseWorldPosition = window.mapPixelToCoords(Mouse::getPosition(), mainView);

			//Set crosshair to the mouse world location
			spriteCrosshair.setPosition(mouseWorldPosition.x, mouseWorldPosition.y);

			//update player
			player.update(dtAsSeconds, Mouse::getPosition());
			Vector2f playerPosition(player.getCenter());
			mainView.setCenter(player.getCenter());

			//loop through each zombie and update them if alive
			for (int i = 0; i < numZombies; i++) 
			{
				if (zombies[i].isAlive()) 
				{
					zombies[i].update(dt.asSeconds(), playerPosition);
				}
			}

			//loop through each active bullet that is in flight
			for (int i = 0; i < 100; i++)
			{
				if (bullets[i].isInFlight())
				{
					bullets[i].update(dtAsSeconds);
				}
			}

		}

		/*
		**************
		DRAW THE SCENE
		**************
		*/

		if (state == State::PLAYING)
		{
			window.clear();

			window.setView(mainView);

			window.draw(background, &textureBackground);
			for (int i = 0; i < numZombies; i++)
			{
				window.draw(zombies[i].getSprite());
			}
			for (int i = 0; i < 100; i++)
			{
				if (bullets[i].isInFlight())
				{
					window.draw(bullets[i].getShape());
				}
			}
			window.draw(player.getSprite());
			window.draw(spriteCrosshair);

		}
		if (state == State::LEVELING_UP)
		{

		}
		if (state == State::PAUSED)
		{

		}
		if (state == State::GAME_OVER)
		{

		}

		window.display();
	}// end of game loop


	delete[] zombies;

	return 0;
}