#include "Player.h"

#include <algorithm>

namespace
{
	// The visible sprite occupies this region of the supplied 1024x512 PNG.
	constexpr int SpriteLeft = 256;
	constexpr int SpriteTop = 96;
	constexpr int SpriteWidth = 543;
	constexpr int SpriteHeight = 351;
	constexpr float SpriteScale = 0.08f;
	constexpr float MarkerPadding = 0.0f;
}

Player::Player() = default;

Player::Player(const sf::Texture& texture, const sf::RenderWindow& window, float startX)
	: mWindowWidth(static_cast<float>(window.getSize().x)),
	  mWindowHeight(static_cast<float>(window.getSize().y))
{
	mSprite.setTexture(texture);
	mSprite.setTextureRect(sf::IntRect(SpriteLeft, SpriteTop, SpriteWidth, SpriteHeight));
	mSprite.setScale(SpriteScale, SpriteScale);

	const float maxX = std::max(0.0f, mWindowWidth - mSprite.getGlobalBounds().width);
	mStartX = std::max(0.0f, std::min(startX, maxX));
	mSprite.setPosition(mStartX, mWindowHeight * 0.8f - mSprite.getGlobalBounds().height);

	mMarker.setFillColor(sf::Color::Transparent);
	mMarker.setOutlineColor(sf::Color::Cyan);
	mMarker.setOutlineThickness(0.0f);
	updateMarker();
}

void Player::updateMarker()
{
	const sf::FloatRect bounds = mSprite.getGlobalBounds();
	mMarker.setSize({ bounds.width + MarkerPadding * 2.0f,
	                  bounds.height + MarkerPadding * 2.0f });
	mMarker.setPosition(bounds.left - MarkerPadding, bounds.top - MarkerPadding);
}

void Player::moveLeft()
{
	const float nextX = std::max(0.0f, mSprite.getPosition().x - mSpeed);
	mSprite.setPosition(nextX, mSprite.getPosition().y);
	updateMarker();
}

void Player::moveRight()
{
	const float maxX = std::max(0.0f, mWindowWidth - mSprite.getGlobalBounds().width);
	const float nextX = std::min(maxX, mSprite.getPosition().x + mSpeed);
	mSprite.setPosition(nextX, mSprite.getPosition().y);
	updateMarker();
}

void Player::setSpeed(float speed)
{
	mSpeed = std::max(0.0f, speed);
}

float Player::getSpeed() const
{
	return mSpeed;
}

float Player::getX() const
{
	return mSprite.getPosition().x;
}

const sf::Sprite& Player::getSprite() const
{
	return mSprite;
}

const sf::RectangleShape& Player::getMarker() const
{
	return mMarker;
}

sf::FloatRect Player::getBounds() const
{
	return mSprite.getGlobalBounds();
}

void Player::reset(float startX)
{
	const float maxX = std::max(0.0f, mWindowWidth - mSprite.getGlobalBounds().width);
	mStartX = std::max(0.0f, std::min(startX, maxX));
	mSprite.setPosition(mStartX, mWindowHeight * 0.8f - mSprite.getGlobalBounds().height);
	updateMarker();
}
