#include "stdafx.h"

#include "VectorMath.h"

using namespace std;

void CalcReturns(const vector<vector<float>>& vectors, const vector<float>& weights, vector<float>& returns)
{
	for (size_t i = 0; i < returns.size(); ++i)
	{
		float sum = 0.0f;
		for (size_t j = 0; j < weights.size(); ++j)
			sum += vectors[i][j] * weights[j];
		returns[i] = sum;
	}
}

void CalcReturnsNoReinvestment(const vector<vector<float>>& vectors, const vector<float>& weights, vector<float>& returns)
{
  vector<float> allocations = weights;
  float prevSum = 1.0f;
	
  for (size_t i = 0; i < returns.size(); ++i)
	{
    auto allocSum = Sum(allocations);

		float sum = 0.0f;
    for (size_t j = 0; j < weights.size(); ++j)
    {
      sum += vectors[i][j] * allocations[j];
      allocations[j] *= 1 + vectors[i][j];
    }
		returns[i] = sum/allocSum - 1;
	}
}

float Sum(const vector<float>& x)
{
	auto sum = 0.0f;
	for (auto r : x)
		sum += r;
	return sum;
}

float Mean(const vector<float>& x)
{
	return Sum(x) / x.size();
}

float vol(const vector<float>& x)
{
	const auto sum = Sum(x);
	const auto count = x.size();
	const auto mean = sum / count;
	auto sum2 = 0.0f;
	for (auto f : x) {
		const auto dev = f - mean;
		sum2 += dev*dev;
	}
	return sqrt(sum2 / count);
}

float Correlation(const vector<float>& a, const vector<float>& b)
{
	if (a.size() != b.size())
		throw runtime_error("mismatched vector sizes");
	float ma = Mean(a);
	float mb = Mean(b);
	float upSum = 0.0f;
	float downSumA = 0.0f, downSumB= 0.0f;
	for (size_t i = 0; i < a.size(); ++i)
	{
		float deva = a[i] - ma;
		float devb = b[i] - mb;
		upSum += deva*devb;
		downSumA += deva*deva;
		downSumB += devb*devb;
	}

	return upSum / sqrt(downSumA*downSumB);
}

ostream& operator<<(ostream& os, const vector<float>& v)
{
  os << '[' << v.size() << "] {";
  for (auto i = v.begin(); i != v.end(); ++i)
  {
    if (i != v.begin())
      os << ", ";
    os << *i;
  }
  os << '}';
  return os;
}

NormStats NormStats::normalize(const int normalization_count) const
{
	return NormStats(mean*normalization_count, stdev*sqrt(static_cast<float>(normalization_count)), skew, kurt);
}

NormStats getNormStats(const vector<float> &returns)
{
	const auto sum = Sum(returns);
	const auto count = returns.size();
	const auto mean = sum / count;
	auto sum2 = 0.0f;
	auto sum3 = 0.0f;
	auto sum4 = 0.0f;
	for (auto f : returns) {
		const auto dev = f - mean;
		sum2 += dev*dev;
		sum3 += dev*dev*dev;
		sum4 += dev*dev*dev*dev;
	}
	const auto stdev = sqrt(sum2 / count);
	const auto skew = sum3 / (count*stdev*stdev*stdev);
	const auto kurt = sum4 / (count*stdev*stdev*stdev*stdev);

	return NormStats(mean, stdev, skew, kurt);
}
