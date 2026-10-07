#include "Brain.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace
{
	constexpr std::size_t InputCount = BrainInputCount;
	constexpr std::size_t OutputCount = 3;
	constexpr std::size_t WeightsPerOutput = InputCount + 1; // Includes bias.
}

Brain::Brain(std::mt19937& rng)
	: weights(OutputCount * WeightsPerOutput)
{
	std::uniform_real_distribution<float> initialWeight(-1.0f, 1.0f);
	for (float& weight : weights)
		weight = initialWeight(rng);
}

std::array<float, 3> Brain::think(const BrainInputs& inputs) const
{
	std::array<float, OutputCount> scores{};
	for (std::size_t output = 0; output < OutputCount; ++output)
	{
		const std::size_t offset = output * WeightsPerOutput;
		float score = weights[offset + InputCount]; // Bias.
		for (std::size_t input = 0; input < InputCount; ++input)
			score += inputs[input] * weights[offset + input];
		scores[output] = std::tanh(score);
	}
	return scores;
}

Brain Brain::crossover(const Brain& other, std::mt19937& rng) const
{
	if (weights.size() != other.weights.size())
		throw std::invalid_argument("Cannot cross over brains with different weight counts.");

	Brain child(*this);
	std::bernoulli_distribution chooseThisParent(0.5);
	for (std::size_t i = 0; i < weights.size(); ++i)
		child.weights[i] = chooseThisParent(rng) ? weights[i] : other.weights[i];
	child.fitness = 0.0f;
	return child;
}

void Brain::mutate(std::mt19937& rng, float mutationChance)
{
	std::bernoulli_distribution shouldMutate(mutationChance);
	std::normal_distribution<float> mutationAmount(0.0f, 0.2f);
	for (float& weight : weights)
	{
		if (shouldMutate(rng))
			weight = std::max(-3.0f, std::min(3.0f, weight + mutationAmount(rng)));
	}
}

void Brain::setFitness(float newFitness)
{
	fitness = newFitness;
}

float Brain::getFitness() const
{
	return fitness;
}
