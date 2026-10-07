#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>

class Player
{
public:
	Player();
	Player(const sf::Texture& texture, const sf::RenderWindow& window, float startX);

	void moveLeft();
	void moveRight();
	void setSpeed(float speed);
	float getSpeed() const;
	float getX() const;
	const sf::Sprite& getSprite() const;
	const sf::RectangleShape& getMarker() const;
	sf::FloatRect getBounds() const;
	void reset(float startX);

private:
	void updateMarker();

	float mWindowWidth = 0.0f;
	float mWindowHeight = 0.0f;
	float mStartX = 0.0f;
	float mSpeed = 4.0f;
	sf::Sprite mSprite;
	sf::RectangleShape mMarker;
};

#endif // PLAYER_H
