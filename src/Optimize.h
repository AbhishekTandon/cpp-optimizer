#pragma once

#include "Constraint.h"
#include "OptimizationTargets.h"

struct EfficientFrontierPoint
{
	const ReturnStats stats;
	const std::vector<float> weights;

	EfficientFrontierPoint(const ReturnStats& stats, const std::vector<float>& weights)
		: stats(stats),
		weights(weights)
	{
	}
};

std::vector<float> optimize(const std::vector<Constraint>& constraints, const std::vector<std::vector<float>>& returns, std::function<float(const std::vector<float>&)> returnsFunc, bool maximize, std::ostream& log);
std::vector<EfficientFrontierPoint> buildEfficientFrontier(
	const std::vector<Constraint> &constraints, 
	const std::vector<std::vector<float>> &returns, 
	const std::vector<std::vector<float>> &benchmarks, 
	size_t normalization_count, 
	const ReturnStats &factors);
