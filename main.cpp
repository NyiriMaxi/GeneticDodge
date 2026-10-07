#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>

#include "Player.h"
#include "Obstacles.h"
#include "Brain.h"

namespace
{
	constexpr std::size_t PopulationSize = 50;
	constexpr std::size_t EliteCount = 10;
	constexpr unsigned int RandomSeed = 42;

	struct ObstacleThreat
	{
		float timeToPlayerSeconds;
		float horizontalOffset;
		float normalizedTimeToPlayer;
	};

	constexpr int PredictionHorizonFrames = 36;

	float predictCollisionRisk(int direction,
	                           const Player& player,
	                           const std::vector<Obstacle>& obstacles,
	                           float windowWidth)
	{
		const sf::FloatRect currentPlayer = player.getBounds();
		const float maxPlayerX = std::max(0.0f, windowWidth - currentPlayer.width);
		float risk = 0.0f;

		for (const Obstacle& obstacle : obstacles)
		{
			const sf::FloatRect currentObstacle = obstacle.getBounds();
			for (int frame = 1; frame <= PredictionHorizonFrames; ++frame)
			{
				const float predictedX = std::clamp(
					currentPlayer.left + direction * player.getSpeed() * frame,
					0.0f, maxPlayerX);
				sf::FloatRect predictedPlayer(
					predictedX, currentPlayer.top, currentPlayer.width, currentPlayer.height);

				sf::FloatRect predictedObstacle = currentObstacle;
				predictedObstacle.top += obstacle.getSpeedPerFrame() * frame;
				if (predictedPlayer.intersects(predictedObstacle))
				{
					// Earlier impacts and multiple threatened obstacles raise the risk score.
					risk += 1.0f + static_cast<float>(PredictionHorizonFrames - frame) /
						static_cast<float>(PredictionHorizonFrames);
					break;
				}
			}
		}

		return risk;
	}

	int chooseSaferDirection(int preferredDirection,
	                         const Player& player,
	                         const std::vector<Obstacle>& obstacles,
	                         float windowWidth)
	{
		const float preferredRisk = predictCollisionRisk(
			preferredDirection, player, obstacles, windowWidth);
		if (preferredRisk == 0.0f)
			return preferredDirection;

		int safestDirection = preferredDirection;
		float lowestRisk = preferredRisk;
		for (const int candidate : { 0, -1, 1 })
		{
			const float candidateRisk = predictCollisionRisk(
				candidate, player, obstacles, windowWidth);
			if (candidateRisk < lowestRisk ||
				(candidateRisk == lowestRisk && candidateRisk == 0.0f && candidate == 0))
			{
				lowestRisk = candidateRisk;
				safestDirection = candidate;
			}
		}
		return safestDirection;
	}
}

int main()
{
	std::mt19937 rng(RandomSeed);
	std::vector<Brain> brains;
	brains.reserve(PopulationSize);
	for (std::size_t i = 0; i < PopulationSize; ++i)
		brains.emplace_back(rng);

	std::vector<bool> alive(PopulationSize, true);
	std::vector<float> survivalTimes(PopulationSize, 0.0f);

	sf::RenderWindow window(sf::VideoMode(1700, 700), "Genetic Dodge");
	window.setKeyRepeatEnabled(false);
	window.setFramerateLimit(60);
	const float windowWidth = static_cast<float>(window.getSize().x);
	const float windowHeight = static_cast<float>(window.getSize().y);

	sf::Texture playerTexture;
	if (!playerTexture.loadFromFile("../Images/PlayerSprite.png"))
	{
		std::cerr << "Could not load ../Images/PlayerSprite.png. Run from the project directory.\n";
		return 1;
	}

	std::vector<Player> players;
	players.reserve(PopulationSize);
	for (std::size_t i = 0; i < PopulationSize; ++i)
	{
		const float startX = (windowWidth - 32.0f) * static_cast<float>(i) /
			static_cast<float>(PopulationSize - 1);
		players.emplace_back(playerTexture, window, startX);
	}

	std::vector<Obstacle> obstacles;
	sf::Clock spawnClock;
	sf::Clock generationClock;

	sf::Font font;
	if (!font.loadFromFile("Sakire.ttf"))
	{
		std::cerr << "Could not load Sakire.ttf. Run from the project directory.\n";
		return 1;
	}

	sf::Text scoreboard;
	scoreboard.setFont(font);
	scoreboard.setCharacterSize(32);
	scoreboard.setPosition(16.0f, 12.0f);

	std::size_t generation = 1;
	float bestEverSurvival = 0.0f;
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			else if (event.type == sf::Event::KeyPressed &&
			         event.key.code == sf::Keyboard::Escape)
				window.close();
		}

		if (!window.isOpen())
			break;

		if (spawnClock.getElapsedTime().asSeconds() >= 0.5f &&
		    obstacles.size() < BrainObstacleSlots)
		{
			obstacles.emplace_back(windowWidth, rng);
			spawnClock.restart();
		}

		for (Obstacle& obstacle : obstacles)
			obstacle.update();

		obstacles.erase(
			std::remove_if(obstacles.begin(), obstacles.end(),
				[windowHeight](const Obstacle& obstacle)
				{
					return obstacle.getBounds().top > windowHeight;
				}),
			obstacles.end());

		const float elapsed = generationClock.getElapsedTime().asSeconds();
		window.clear();

		for (const Obstacle& obstacle : obstacles)
			window.draw(obstacle);

		for (std::size_t i = 0; i < PopulationSize; ++i)
		{
			if (!alive[i])
				continue;

			const sf::FloatRect playerBounds = players[i].getBounds();
			const float playerCenterX = playerBounds.left + playerBounds.width / 2.0f;
			BrainInputs brainInputs;
			// 2.0 is outside the normalized [-1, 1] feature range and marks an empty slot.
			brainInputs.fill(2.0f);
			brainInputs[0] = (playerCenterX / windowWidth) * 2.0f - 1.0f;

			std::array<ObstacleThreat, BrainObstacleSlots> threats{};
			std::size_t threatCount = 0;
			for (const Obstacle& obstacle : obstacles)
			{
				const sf::FloatRect bounds = obstacle.getBounds();
				const float obstacleCenterX = bounds.left + bounds.width / 2.0f;
				const float obstacleBottom = bounds.top + bounds.height;
				const float speedPixelsPerSecond =
					std::max(0.001f, obstacle.getSpeedPerFrame() * 60.0f);
				const float timeToPlayer =
					(playerBounds.top - obstacleBottom) / speedPixelsPerSecond;
				const float fullScreenTravelTime = windowHeight / speedPixelsPerSecond;

				threats[threatCount++] = {
					timeToPlayer,
					std::clamp((obstacleCenterX - playerCenterX) / windowWidth, -1.0f, 1.0f),
					std::clamp(timeToPlayer / fullScreenTravelTime, -1.0f, 1.0f)
				};
			}

			std::sort(threats.begin(), threats.begin() + threatCount,
				[](const ObstacleThreat& lhs, const ObstacleThreat& rhs)
				{
					const float lhsDistance = std::abs(lhs.timeToPlayerSeconds);
					const float rhsDistance = std::abs(rhs.timeToPlayerSeconds);
					if (lhsDistance == rhsDistance)
						return lhs.horizontalOffset < rhs.horizontalOffset;
					return lhsDistance < rhsDistance;
				});

			for (std::size_t threat = 0; threat < threatCount; ++threat)
			{
				const std::size_t featureIndex = 1 + threat * 2;
				brainInputs[featureIndex] = threats[threat].horizontalOffset;
				brainInputs[featureIndex + 1] = threats[threat].normalizedTimeToPlayer;
			}

			const std::array<float, 3> actionScores = brains[i].think(brainInputs);
			const std::size_t action = static_cast<std::size_t>(
				std::max_element(actionScores.begin(), actionScores.end()) - actionScores.begin());
			const int preferredDirection = action == 0 ? -1 : (action == 2 ? 1 : 0);
			const int direction = chooseSaferDirection(
				preferredDirection, players[i], obstacles, windowWidth);

			if (direction < 0)
				players[i].moveLeft();
			else if (direction > 0)
				players[i].moveRight();

			for (const Obstacle& obstacle : obstacles)
			{
				if (obstacle.getBounds().intersects(players[i].getBounds()))
				{
					alive[i] = false;
					survivalTimes[i] = elapsed;
					break;
				}
			}

			if (alive[i])
			{
				window.draw(players[i].getMarker());
				window.draw(players[i].getSprite());
			}
		}

		const bool anyAlive = std::any_of(alive.begin(), alive.end(),
			[](bool isAlive) { return isAlive; });

		if (!anyAlive)
		{
			for (std::size_t i = 0; i < PopulationSize; ++i)
			{
				brains[i].setFitness(survivalTimes[i]);
				bestEverSurvival = std::max(bestEverSurvival, survivalTimes[i]);
			}

			std::sort(brains.begin(), brains.end(),
				[](const Brain& lhs, const Brain& rhs)
				{
					return lhs.getFitness() > rhs.getFitness();
				});

			std::vector<Brain> nextGeneration;
			nextGeneration.reserve(PopulationSize);
			for (std::size_t i = 0; i < EliteCount; ++i)
				nextGeneration.push_back(brains[i]);

			std::uniform_int_distribution<std::size_t> selectElite(0, EliteCount - 1);
			for (std::size_t i = EliteCount; i < PopulationSize; ++i)
			{
				const Brain& parent1 = brains[selectElite(rng)];
				const Brain& parent2 = brains[selectElite(rng)];
				Brain child = parent1.crossover(parent2, rng);
				child.mutate(rng);
				nextGeneration.push_back(child);
			}

			brains.swap(nextGeneration);
			std::fill(alive.begin(), alive.end(), true);
			std::fill(survivalTimes.begin(), survivalTimes.end(), 0.0f);
			std::vector<std::size_t> laneOrder(PopulationSize);
			std::iota(laneOrder.begin(), laneOrder.end(), 0);
			std::shuffle(laneOrder.begin(), laneOrder.end(), rng);
			for (std::size_t i = 0; i < PopulationSize; ++i)
			{
				const float startX = (windowWidth - 32.0f) *
					static_cast<float>(laneOrder[i]) / static_cast<float>(PopulationSize - 1);
				players[i].reset(startX);
			}

			obstacles.clear();
			spawnClock.restart();
			generationClock.restart();
			++generation;
		}

		std::ostringstream generationTimer;
		generationTimer << std::fixed << std::setprecision(1)
			<< generationClock.getElapsedTime().asSeconds();
		scoreboard.setString("Generation: " + std::to_string(generation) +
			"    Alive: " + std::to_string(std::count(alive.begin(), alive.end(), true)) +
			"    Gen time: " + generationTimer.str() + " s" +
			"    Best survival: " + std::to_string(static_cast<int>(bestEverSurvival)) + " s");
		window.draw(scoreboard);
		window.display();
	}

	return 0;
}
