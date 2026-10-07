#include "Obstacles.h"

#include <algorithm>

void Obstacle::draw(sf::RenderTarget& window, sf::RenderStates states) const
{
	window.draw(mShape, states);
}

Obstacle::Obstacle()
	: mShape({ 32.0f, 32.0f })
{
	mShape.setFillColor(sf::Color::Green);
}

Obstacle::Obstacle(float windowWidth, std::mt19937& rng)
	: mShape({ 32.0f, 32.0f })
{
	mShape.setFillColor(sf::Color::Green);
	const float maxX = std::max(0.0f, windowWidth - mShape.getSize().x);
	std::uniform_real_distribution<float> randomX(0.0f, maxX);
	mShape.setPosition(randomX(rng), 0.0f);
}

void Obstacle::update()
{
	mShape.move(0.0f, mSpeed);
}

void Obstacle::IncreaseSpeed()
{
	mSpeed += 0.5f;
}

sf::FloatRect Obstacle::getBounds() const
{
	return mShape.getGlobalBounds();
}

float Obstacle::getSpeedPerFrame() const
{
	return mSpeed;
}
