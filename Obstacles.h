#ifndef OBSTACLES_H
#define OBSTACLES_H

#include <random>
#include <SFML/Graphics.hpp>

class Obstacle : public sf::Drawable
{
public:
	Obstacle();
	Obstacle(float windowWidth, std::mt19937& rng);
	void IncreaseSpeed();
	void update();
	sf::FloatRect getBounds() const;
	float getSpeedPerFrame() const;

protected:
	void draw(sf::RenderTarget& window, sf::RenderStates states) const override;

private:
	sf::RectangleShape mShape;
	float mSpeed = 5.0f;
};

#endif // OBSTACLES_H
