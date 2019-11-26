#ifndef player_h
#define player_h

#include <SFML/Graphics.hpp>
#include <math.h>
#include "TextureHolder.h"

using namespace sf;

class Player
{
private:
	const float START_SPEED = 200;
	const float START_HEALTH = 100;

	//where is the player?
	Vector2f m_Position;

	//player sprite
	Sprite m_Sprite;

	//player texture
	Texture m_Texture;

	//what is the screen resolution?
	Vector2f m_Resolution;

	//how big is the current area?
	IntRect m_Arena;

	//how big is each tile of the arena
	int m_TileSize;

	//which directions is the player currently moving in?
	bool m_UpPressed, m_DownPressed, m_LeftPressed, m_RightPressed;

	//how much health has the player got?
	int m_Health;

	// what is the maximum health the player can have
	int m_MaxHealth;

	//when was the player last hit?
	Time m_LastHit;
	
	//speed in pixels per second
	float m_Speed;

public:
	Player();

	void spawn(IntRect arena, Vector2f resolution, int tileSize);

	//called at end of game
	void resetPlayerStats();

	//handle the player getting hit by a zombie
	bool hit(Time timeHit);

	//how long since player hit?
	Time getLastHitTime();

	//where is the player?
	FloatRect getPosition();

	//where is the center of the player?
	Vector2f getCenter();

	//which angle is the player facing
	float getRotation();

	//send a copy of the sprite to mail
	Sprite getSprite();

	//how much health has the player currently got?
	int getHealth();

	//move the player
	void moveLeft(), moveRight(), moveUp(), moveDown();

	//stop the player moving in a specific direction
	void stopLeft(), stopRight(), stopUp(), stopDown();

	//called every frame
	void update(float elapsedTime, Vector2i mousePosition);

	//give the player a speed boost
	void upgradeSpeed();
	
	//give the player some health
	void upgradeHealth();

	//increase the maximum amount of health the player can have
	void increaseHealthLevel(int amount);
};
#endif
