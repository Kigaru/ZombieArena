#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "player.h"
#include "ZombieArena.h"
#include <iostream>
#include "TextureHolder.h"
#include "bullet.h"
#include "pickup.h"
#include <sstream>
#include <fstream>
#include <string>
#include "../resources/discord-api/discord.h"
using namespace sf;

int main() 
{
	//discord code
	discord::Core* core{};
	discord::Core::Create(475090671884042251, DiscordCreateFlags_Default, &core);

	discord::Activity activity{};
	activity.SetState("test123");
	float frameUpdate = 0;
	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {}); //TO UPDATE THE STATE USE THIS
	//end of discord

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

	//create couple of pick ups
	Pickup healthPickup(1);
	Pickup ammoPickup(2);


	//about the game
	int score = 0;
	int hiScore = 0;

	Sprite spriteGameOver;
	Texture textureGameOver = TextureHolder::GetTexture("resources/graphics/background.png");
	spriteGameOver.setTexture(textureGameOver);
	spriteGameOver.setPosition(0,0);

	//image is 1080p - needs scaling for large displays
	spriteGameOver.setScale(resolution.x / 1920, resolution.y / 1080);

	View hudView(FloatRect(0, 0, resolution.x, resolution.y));
	Sprite spriteAmmoIcon;
	Texture textureAmmoIcon = TextureHolder::GetTexture("resources/graphics/ammo_icon.png");
	spriteAmmoIcon.setTexture(textureAmmoIcon);
	spriteAmmoIcon.setPosition(20, resolution.y - 200);

	Font font;
	font.loadFromFile("resources/fonts/zombiecontrol.ttf");

	//PausedText
	Text pausedText;
	pausedText.setFont(font);
	pausedText.setCharacterSize(155);
	pausedText.setFillColor(Color::White);
	pausedText.setString("Press Enter\nto continue...");
	
	//Place in the middle of the screen
	FloatRect pausedRect = pausedText.getLocalBounds();
	pausedText.setOrigin(pausedRect.left + pausedRect.width / 2, pausedRect.top + pausedRect.height / 2);
	pausedText.setPosition(resolution.x / 2, resolution.y / 2);

	// Game Over
	Text gameOverText;
	gameOverText.setFont(font);
	gameOverText.setCharacterSize(125);
	gameOverText.setFillColor(sf::Color::White);
	gameOverText.setString("Press Enter to play");

	FloatRect gameOverRect = gameOverText.getLocalBounds();
	gameOverText.setOrigin(gameOverRect.left + gameOverRect.width / 2.0f, gameOverRect.top + gameOverRect.height / 2.0f);
	gameOverText.setPosition(resolution.x / 2, resolution.y / 2);

	// Levelling up
	Text levelUpText;
	levelUpText.setFont(font);
	levelUpText.setCharacterSize(80);
	levelUpText.setFillColor(Color::White);
	levelUpText.setPosition(150, 250);
	std::stringstream levelUpStream;
	levelUpStream <<
		"1- Increased rate of fire" <<
		"\n2- Increased clip size(next reload)" <<
		"\n3- Increased max health" <<
		"\n4- Increased run speed" <<
		"\n5- More and better health pickups" <<
		"\n6- More and better ammo pickups";
	levelUpText.setString(levelUpStream.str());

	FloatRect levelUpRect = levelUpText.getLocalBounds();
	levelUpText.setOrigin(levelUpRect.left + levelUpRect.width / 2.0f, levelUpRect.top + levelUpRect.height / 2.0f);
	levelUpText.setPosition(resolution.x / 2, resolution.y / 2);

	// Ammo
	Text ammoText;
	ammoText.setFont(font);
	ammoText.setCharacterSize(55);
	ammoText.setFillColor(Color::White);
	ammoText.setPosition(200, resolution.y - 200);

	// Score
	Text scoreText;
	scoreText.setFont(font);
	scoreText.setCharacterSize(55);
	scoreText.setFillColor(Color::White);
	scoreText.setPosition(20, 0);

	//load the high score from a text file
	ifstream inputFile("resources/gamedata/scores.txt");
	if (inputFile.is_open())
	{
		inputFile >> hiScore;
		inputFile.close();
	}

	// Hi Score
	Text hiScoreText;
	hiScoreText.setFont(font);
	hiScoreText.setCharacterSize(55);
	hiScoreText.setFillColor(Color::White);
	hiScoreText.setPosition(resolution.x - 400, 0);
	std::stringstream s;
	s << "Hi Score:" << hiScore;
	hiScoreText.setString(s.str());

	// Zombies remaining
	Text zombiesRemainingText;
	zombiesRemainingText.setFont(font);
	zombiesRemainingText.setCharacterSize(55);
	zombiesRemainingText.setFillColor(Color::White);
	zombiesRemainingText.setPosition(resolution.x - 400, resolution.y - 200);
	zombiesRemainingText.setString("Zombies: 100");

	// Wave number
	int wave = 0;
	Text waveNumberText;
	waveNumberText.setFont(font);
	waveNumberText.setCharacterSize(55);
	waveNumberText.setFillColor(Color::White);
	waveNumberText.setPosition(resolution.x*0.66, resolution.y - 200);
	waveNumberText.setString("Wave: 0");

	// Health bar
	RectangleShape healthBar;
	healthBar.setFillColor(Color::Red);
	healthBar.setPosition(resolution.x*0.33, resolution.y - 200);

	//when did we last update the HUD
	int framesSinceLastHUDUPdate = 0;
	int fpsMeasurementFrameInterval = 1000;

	//sound stuff
	SoundBuffer hitBuffer;
	hitBuffer.loadFromFile("resources/sound/hit.wav");
	Sound hit;
	hit.setBuffer(hitBuffer);


	// Prepare the splat sound
	SoundBuffer splatBuffer;
	splatBuffer.loadFromFile("resources/sound/splat.wav");
	sf::Sound splat;
	splat.setBuffer(splatBuffer);

	// Prepare the shoot sound
	SoundBuffer shootBuffer;
	shootBuffer.loadFromFile("resources/sound/shoot.wav");
	Sound shoot;
	shoot.setBuffer(shootBuffer);

	// Prepare the reload sound
	SoundBuffer reloadBuffer;
	reloadBuffer.loadFromFile("resources/sound/reload.wav");
	Sound reload;
	reload.setBuffer(reloadBuffer);

	// Prepare the failed sound
	SoundBuffer reloadFailedBuffer;
	reloadFailedBuffer.loadFromFile("resources/sound/reload_failed.wav");
	Sound reloadFailed;
	reloadFailed.setBuffer(reloadFailedBuffer);

	// Prepare the powerup sound
	SoundBuffer powerupBuffer;
	powerupBuffer.loadFromFile("resources/sound/powerup.wav");
	Sound powerup;
	powerup.setBuffer(powerupBuffer);

	// Prepare the pickup sound
	SoundBuffer pickupBuffer;
	pickupBuffer.loadFromFile("resources/sound/pickup.wav");
	Sound pickup;
	pickup.setBuffer(pickupBuffer);


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
							reload.play();
						}
						else if (bulletsSpare > 0)
						{
							//Only few bullets left
							bulletsInClip = bulletsSpare;
							bulletsSpare = 0;
							reload.play();
						}
						else
						{
							//More here later - reload failed
							reloadFailed.play();
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
					wave = 0;
					score = 0;

					//prepare gun and ammo for next game;
					currentBullet = 0;
					bulletsSpare = 24;
					bulletsInClip = 6;
					clipSize = 6;
					fireRate = 1;
					player.resetPlayerStats();
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
						//increase fire rate
						fireRate++;
						state = State::PLAYING;
					}
					else if (event.key.code == Keyboard::Num2)
					{
						//increase clip size
						clipSize += clipSize;
						state = State::PLAYING;
					}
					else if (event.key.code == Keyboard::Num3)
					{
						//increase health
						player.upgradeHealth();
						state = State::PLAYING;
					}
					else if (event.key.code == Keyboard::Num4)
					{
						//increase speed
						player.upgradeSpeed();
						state = State::PLAYING;
					}
					else if (event.key.code == Keyboard::Num5)
					{
						//increase health pickup
						healthPickup.upgrade();
						state = State::PLAYING;
					}
					else if (event.key.code == Keyboard::Num6)
					{
						//increase ammo pickup
						ammoPickup.upgrade();
						state = State::PLAYING;
					}
					if (state == State::PLAYING)
					{
						//increase wave number
						wave++;
						//prep level
						arena.width = 500 * wave;
						arena.height = 500 * wave;
						arena.left = 0;
						arena.top = 0;

						int tileSize = createBackground(background, arena);

						//spawn player in the middle
						player.spawn(arena, resolution, tileSize);

						//configure the pickups
						healthPickup.setArena(arena);
						ammoPickup.setArena(arena);

						numZombies = 5 * wave;
						delete[] zombies;
						zombies = createHorde(numZombies, arena);
						numZombiesAlive = numZombies;

						//play the powerup sound
						powerup.play();

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
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
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
					shoot.play();
					bulletsInClip--;
				}//end fire bullet
			}
		}// end is playing input
		

		/*
		****************
		UPDATE THE FRAME
		****************
		*/
		Time dt = clock.restart();
		gameTimeTotal += dt;
		float dtAsSeconds = dt.asSeconds();
		if (state == State::PLAYING)
		{

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

			//loop through each pickup
			healthPickup.update(dtAsSeconds);
			ammoPickup.update(dtAsSeconds);
		


			// Collision detection
			// Have any zombies been shot?
			for (int i = 0; i < 100; i++)
			{
				for (int j = 0; j < numZombies; j++)
				{
					if (bullets[i].isInFlight() &&
						zombies[j].isAlive())
					{
						if (bullets[i].getPosition().intersects
						(zombies[j].getPosition()))
						{
							// Stop the bullet
							bullets[i].stop();

							// Register the hit and see if it was a kill
							if (zombies[j].hit()) {
								// Not just a hit but a kill too
								score += 10;
								if (score >= hiScore)
								{
									hiScore = score;
								}

								numZombiesAlive--;

								// When all the zombies are dead (again)
								if (numZombiesAlive == 0) {
									state = State::LEVELING_UP;
								}
							}
							splat.play();
						}
					}

				}
			}// End zombie being shot


			// Have any zombies touched the player?
			for (int i = 0; i < numZombies; i++)
			{
				if (player.getPosition().intersects
				(zombies[i].getPosition()) && zombies[i].isAlive())
				{

					if (player.hit(gameTimeTotal))
					{
						hit.play();
					}

					if (player.getHealth() <= 0)
					{
						state = State::GAME_OVER;

						ofstream outputFile("resources/gamedata/scores.txt");
						outputFile << hiScore;
						outputFile.close();
					}
				}
			}// End player touched


			// Has the player touched health pickup?
			if (player.getPosition().intersects
			(healthPickup.getPosition()) && healthPickup.isSpawned())
			{
				player.increaseHealthLevel(healthPickup.gotIt());
				pickup.play();
			}// End player touch health


			// Has the player touched ammo pickup?
			if (player.getPosition().intersects
			(ammoPickup.getPosition()) && ammoPickup.isSpawned())
			{
				bulletsSpare += ammoPickup.gotIt();
				reload.play();
			} // End player touch ammo


			//size up the health bar
			healthBar.setSize(Vector2f(player.getHealth() * 3, 70));

			//increment the number of frames since the last HUD calculation
			framesSinceLastHUDUPdate++;

			//calculate fps every fpsMeasurementFrameInterval frames
			if (framesSinceLastHUDUPdate > fpsMeasurementFrameInterval)
			{
				stringstream ssAmmo;
				stringstream ssScore;
				stringstream ssHiScore;
				stringstream ssWave;
				stringstream ssZombiesAlive;

				ssAmmo << bulletsInClip << "/" << bulletsSpare;
				ammoText.setString(ssAmmo.str());

				ssScore << "Score:" << score;
				scoreText.setString(ssScore.str());

				ssHiScore << "Hi Score:" << hiScore;
				hiScoreText.setString(ssHiScore.str());

				ssWave << "Wave:" << wave;
				waveNumberText.setString(ssWave.str());

				ssZombiesAlive << "Zombies:" << numZombiesAlive;
				zombiesRemainingText.setString(ssZombiesAlive.str());

				framesSinceLastHUDUPdate = 0;
			} //end hud update
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

			if (ammoPickup.isSpawned())
			{
				window.draw(ammoPickup.getSprite());
			}
			if (healthPickup.isSpawned())
			{
				window.draw(healthPickup.getSprite());
			}

			window.draw(spriteCrosshair);

			//set & draw all hud elements
			window.setView(hudView);
			window.draw(spriteAmmoIcon);
			window.draw(ammoText);
			window.draw(scoreText);
			window.draw(hiScoreText);
			window.draw(healthBar);
			window.draw(waveNumberText);
			window.draw(zombiesRemainingText);
		}
		if (state == State::LEVELING_UP)
		{
			window.draw(spriteGameOver);
			window.draw(levelUpText);
		}
		if (state == State::PAUSED)
		{
			window.draw(pausedText);
		}
		if (state == State::GAME_OVER)
		{
			window.draw(spriteGameOver);
			window.draw(gameOverText);
			window.draw(scoreText);
			window.draw(hiScoreText);
		}

		window.display();



		/*
		*************
		DISCORD STUFF
		*************
		*/
		if (state == State::PLAYING)
		{
			string details = "Wave " + std::to_string(wave);
			activity.SetDetails(details.c_str());
			string state = "Zombies remaining: " + std::to_string(numZombiesAlive);
			activity.SetState(state.c_str());
		}
		if (state == State::LEVELING_UP)
		{
			activity.SetDetails("Powering up...");
			string state = "Wave " + std::to_string(wave);
			activity.SetState(state.c_str());
		}
		if (state == State::PAUSED)
		{
			string details = "Wave " + std::to_string(wave);
			activity.SetDetails(details.c_str());
			activity.SetState("AFK");
		}
		if (state == State::GAME_OVER)
		{
			activity.SetDetails("Game Over");
			string state = "Hi Score " + std::to_string(hiScore);
			activity.SetState(state.c_str());
		}
		core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
		frameUpdate += dt.asMilliseconds();
		if(frameUpdate > 16) 
		{
			frameUpdate = 0;
			core->RunCallbacks();
		}
	}// end of game loop


	delete[] zombies;

	return 0;
}