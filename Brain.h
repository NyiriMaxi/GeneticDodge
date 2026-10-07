#ifndef BRAIN_H
#define BRAIN_H

#include <array>
#include <cstddef>
#include <random>
#include <vector>

inline constexpr std::size_t BrainObstacleSlots = 10;
inline constexpr std::size_t BrainInputCount = 1 + BrainObstacleSlots * 2;
using BrainInputs = std::array<float, BrainInputCount>;

class Brain
{
public:
	explicit Brain(std::mt19937& rng);

	// Player position plus (horizontal offset, time-to-player) for each obstacle slot.
	std::array<float, 3> think(const BrainInputs& inputs) const;
	Brain crossover(const Brain& other, std::mt19937& rng) const;
	void mutate(std::mt19937& rng, float mutationChance = 0.1f);

	void setFitness(float newFitness);
	float getFitness() const;

private:
	// Three outputs, each with three input weights and one bias.
	std::vector<float> weights;
	float fitness = 0.0f;
};

#endif // BRAIN_H
