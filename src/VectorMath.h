#pragma once

void CalcReturns(const std::vector<std::vector<float>>& vectors, const std::vector<float>& weights, std::vector<float>& returns);
float Sum(const std::vector<float>& x);
float Mean(const std::vector<float>& x);
float vol(const std::vector<float>& x);
/// Scale x so that sum of its components=1. Zero sum remains unscaled
void Normalize(std::vector<float>& x);
float Correlation(const std::vector<float>& a, const std::vector<float>& b);
std::ostream& operator<<(std::ostream& os, const std::vector<float>& v);

struct NormStats
{
	const float mean, stdev, skew, kurt;

	NormStats(const float mean, const float stdev, const float skew, const float kurt)
		: mean(mean),
		  stdev(stdev),
		  skew(skew),
		  kurt(kurt)
	{
	}

	NormStats normalize(int normalization_count) const;
};

NormStats getNormStats(const std::vector<float> &returns);