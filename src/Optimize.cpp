#include "stdafx.h"
#include "VectorMath.h"
#include "OptimizationTargets.h"
#include "onullstream.h"
#include "Optimize.h"

using namespace std;

float getWorstResult(bool maximize) {
  return maximize ? -numeric_limits<float>::infinity()
                  : numeric_limits<float>::infinity();
}

// Scale x so that sum of its components=1. Zero sum remains unscaled
void normalize(vector<float> &x, const vector<Constraint> constraints) {
  auto sum = Sum(x);
  if (sum == 0.0f || sum == 1.0f)
    return;

  bool decrease = sum > 1;
  auto overflow = abs(sum - 1);

  // find nearest to constraint value
  // adjust all other values by that smallest difference
  // repeat until overflow is matched
  bool matched;
  do {
    auto minDiff = numeric_limits<float>::infinity();
    size_t count = 0;
    for (size_t i = 0; i < x.size(); ++i) {
      auto diff =
          decrease ? x[i] - constraints[i].min() : constraints[i].max() - x[i];
      if (diff > 0) {
        if (diff < minDiff)
          minDiff = diff;
        ++count;
      }
    }
    assert(count > 0);

    auto diff = overflow / count;
    matched = diff <= minDiff;
    if (!matched) {
      diff = minDiff;
      overflow -= diff * count;
    }
    for (size_t i = 0; i < x.size(); ++i)
      if (decrease ? x[i] > constraints[i].min() : x[i] < constraints[i].max())
        x[i] += decrease ? -diff : diff;
  } while (!matched);
}

void growInternal(vector<float> &weights, const vector<Constraint> &constraints,
                  float sum);
void shrinkInternal(vector<float> &weights,
                    const vector<Constraint> &constraints, float sum);

void optimalWalk(const vector<Constraint> &constraints,
                 const vector<vector<float>> &returns,
                 function<float(const vector<float> &)> func, bool maximize,
                 ostream &log, float initialStep, float accuracy,
                 vector<float> &best, float &bestRes) {
  size_t portCount = returns[0].size();

  // optimization
  auto step = initialStep;
  auto stepUp = vector<float>(portCount);
  auto stepDown = vector<float>(portCount);
  auto next = vector<float>(portCount);
  float nextRes;
  const float worstRes = getWorstResult(maximize);

  while (step > accuracy) {
    nextRes = bestRes;

    for (size_t i = 0; i < portCount; ++i) {
      auto &constraint = constraints[i];
      auto min = constraint.min();
      auto max = constraint.max();

      float stepUpRes;
      auto space = max - best[i];
      if (space > 0) {
        stepUp = best;
        float change;
        if (space > step) {
          stepUp[i] += step;
          change = step;
        } else {
          stepUp[i] = max;
          change = space;
        }
        shrinkInternal(stepUp, constraints, 1 + change);
        stepUpRes = func(stepUp);
      } else
        stepUpRes = worstRes;

      float stepDownRes;
      space = best[i] - min;
      if (space > 0) {
        float change;
        stepDown = best;
        if (space > step) {
          stepDown[i] -= step;
          change = step;
        } else {
          stepDown[i] = min;
          change = space;
        }
        growInternal(stepDown, constraints, 1 - change);
        stepDownRes = func(stepDown);
      } else
        stepDownRes = worstRes;

      if (maximize ? stepUpRes > nextRes || stepDownRes > nextRes
                   : stepUpRes < nextRes || stepDownRes < nextRes) {
        if (maximize ? stepUpRes > stepDownRes : stepUpRes < stepDownRes) {
          next = stepUp;
          nextRes = stepUpRes;
        } else {
          next = stepDown;
          nextRes = stepDownRes;
        }

		//log << "found improvement step=" << step << " port=" << i << " new best=" << nextRes << "\n";
      }
    }

    if (maximize ? nextRes > bestRes : nextRes < bestRes) {
      best = next;
      bestRes = nextRes;
      /*      log << "shifted " << step << " new best=" << bestRes << " ::
         weights: ";
            for (auto w : best)
              log << w << ' ';
            log << "\n";*/
    } else
      step *= 0.5f;
  }
}

void growInternal(vector<float> &weights, const vector<Constraint> &constraints,
                  float sum) {
  while (sum < 0.99999f) {
    auto space = numeric_limits<float>::infinity();
    size_t count = 0;
    for (size_t i = 0; i < weights.size(); ++i) {
      auto &w = weights[i];
      auto &c = constraints[i];
      if (w < c.max()) {
        auto diff = c.max() - w;
        if (diff < space)
          space = diff;
        ++count;
      }
    }
    if (count == 0)
      throw runtime_error("Max constraints are too low");

    auto done = space * count >= 1 - sum;
    auto bump = done ? (1 - sum) / count : space;

    sum = 0.0f;
    for (size_t i = 0; i < weights.size(); ++i) {
      auto &w = weights[i];
      auto &c = constraints[i];
      if (w < c.max())
        w += bump;
      sum += w;
    }

    if (done)
      break;
  }
}

void shrinkInternal(vector<float> &weights,
                    const vector<Constraint> &constraints, float sum) {
  while (sum > 1.00001f) {
    auto space = numeric_limits<float>::infinity();
    size_t count = 0;
    for (size_t i = 0; i < weights.size(); ++i) {
      auto &w = weights[i];
      auto &c = constraints[i];
      if (w > c.min()) {
        auto diff = w - c.min();
        if (diff < space)
          space = diff;
        ++count;
      }
    }
    if (count == 0)
      throw runtime_error("Min constraints are too low");

    auto done = space * count >= sum - 1;
    auto bump = done ? (sum - 1) / count : space;

    sum = 0.0f;
    for (size_t i = 0; i < weights.size(); ++i) {
      auto &w = weights[i];
      auto &c = constraints[i];
      if (w > c.min())
        w -= bump;
      sum += w;
    }

    if (done)
      break;
  }
}

void fixConstraints(vector<float> &weights,
                    const vector<Constraint> &constraints) {
  float sum = 0.f;
  for (size_t i = 0; i < weights.size(); ++i) {
    auto &w = weights[i];
    auto &c = constraints[i];

    if (w > c.max())
      w = c.max();
    else if (w < c.min())
      w = c.min();

    sum += w;
  }

  if (sum < 1)
    growInternal(weights, constraints, sum);
  else if (sum > 1)
    shrinkInternal(weights, constraints, sum);
}

vector<float> optimize(const vector<Constraint> &constraints,
                       const vector<vector<float>> &returns,
                       function<float(const vector<float> &)> returnsFunc,
                       bool maximize, ostream &log) {
  size_t portCount = returns[0].size();

  vector<float> weights(portCount, 1.0f / portCount);
  const int digits = 4;
  const float accuracy = pow(10.0f, -digits - 1);

  vector<float> totals(returns.size());
  auto func = [&](const vector<float> &ws) {
    CalcReturns(returns, ws, totals);
    return returnsFunc(totals);
  };
  float result = func(weights);

  vector<Constraint> noConstraints(portCount, Constraint(true, 0, 1));
  optimalWalk(noConstraints, returns, func, maximize, log, 0.1f, accuracy,
              weights, result);

  log << "before constraints" << weights << " result: " << result << endl;

  fixConstraints(weights, constraints);
  result = func(weights);

  log << "after fixing constraints" << weights << " result: " << result << endl;

  optimalWalk(constraints, returns, func, maximize, log, 0.1f, accuracy,
              weights, result);
  
  log << "after constrained optimization" << weights << " result: " << result << endl;

  return weights;
}

vector<float> optimizeWithConstraints(
	const vector<Constraint>& constraints,
	const vector<vector<float>>& returns,
	const vector<vector<float>>& benchmarks,
	function<float(const ReturnStats&)> factor_function,
	function<float(const ReturnStats&)> constraint_function,
	float constraint_target, float constraint_corridor,
	ostream& log)
{
	size_t portCount = returns[0].size();

	vector<float> weights(portCount, 1.0f / portCount);
	const int digits = 4;
	const float accuracy = pow(10.0f, -digits - 1);

	vector<float> totals(returns.size());
	ReturnStats stats;
	auto wideWalkFunc = [&](const vector<float>& ws)
	{
		CalcReturns(returns, ws, totals);
		GetStats(totals, benchmarks, stats);
		auto factor_result = factor_function(stats);
		auto constraint_result = constraint_function(stats);
		auto constraint_dev = (constraint_result - constraint_target) / constraint_corridor;
		auto constraint_factor = 1.f / (1.f + constraint_dev*constraint_dev);
		return factor_result*constraint_factor;
	};
	float result = wideWalkFunc(weights);

	vector<Constraint> noConstraints(portCount, Constraint(true, 0, 1));
	optimalWalk(noConstraints, returns, wideWalkFunc, true, log, 0.1f, accuracy,
	            weights, result);

	log << "before constraints" << weights << " result: " << result << endl;

	fixConstraints(weights, constraints);
	result = wideWalkFunc(weights);

	log << "after fixing constraints" << weights << " result: " << result << endl;

	optimalWalk(constraints, returns, wideWalkFunc, true, log, 0.1f, accuracy,
	            weights, result);

	log << "after constrained optimization" << weights << " result: " << result << endl;

	auto narrowWalkFunc = [&](const vector<float>& ws)
	{
		CalcReturns(returns, ws, totals);
		GetStats(totals, {}, stats);
		auto factor_result = factor_function(stats);
		auto constraint_result = constraint_function(stats);
		auto constraint_dev = (constraint_result - constraint_target) / (constraint_corridor*0.01);
		auto constraint_factor = 1.f / (1.f + abs(constraint_dev));
		return factor_result*constraint_factor;
	};
	result = narrowWalkFunc(weights);

	optimalWalk(constraints, returns, narrowWalkFunc, true, log, 0.1f, accuracy,
	            weights, result);

	log << "after narrow optimization" << weights << " result: " << result << endl;

	return weights;
}

vector<EfficientFrontierPoint> buildEfficientFrontier(
	const vector<Constraint> &constraints,
	const vector<vector<float>> &returns,
	const vector<vector<float>> &benchmarks,
	const size_t normalizationCount,
	const ReturnStats &factors)
{
	vector<EfficientFrontierPoint> result;
	const auto retCount = returns.size();
	vector<float> totals(retCount);
	float volTarget = 0;
	const float sqrtNormFactor = sqrt(static_cast<float>(normalizationCount));
	for (;;) {
		const auto stdevTarget = volTarget/sqrtNormFactor;

		auto factorFunc = [&](const ReturnStats &stats) {
			return ScaleStats(factors, stats);
		};
		auto constraintFunc = [](const ReturnStats &stats) {
			return stats.stdDeviation;
		};

		const auto weights = optimizeWithConstraints(
			constraints, returns, benchmarks,
			factorFunc, constraintFunc, stdevTarget, 0.01f, onullstream::instance());

		CalcReturns(returns, weights, totals);
		ReturnStats stats;
		GetStats(totals, benchmarks, stats);
		stats.normalize(normalizationCount);
		if (stats.stdDeviation - volTarget < -0.005f)
			break;

		result.push_back(EfficientFrontierPoint(stats, weights));

		volTarget = floor(stats.stdDeviation*100.f + 1.5f)*0.01f;
	}
	return result;
}