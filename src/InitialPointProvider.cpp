#include "stdafx.h"
#include "InitialPointProvider.h"

using namespace std;

InitialPointProvider::InitialPointProvider(const std::vector<Constraint>& constraints, std::vector<float>& vals, int digits, size_t steps)
  : constraints(constraints), vals(vals), indexes(constraints.size()), idx(0)
{
  for (size_t i = 0; i < indexes.size(); ++i)
    indexes[i] = i;
  sort(indexes.begin(), indexes.end(), [&constraints](size_t a, size_t b)
  {
    auto aw = constraints[a].width();
    auto bw = constraints[b].width();
    return aw < bw || (aw == bw && constraints[a].required() && !constraints[b].required());
  });

  dim.reserve(indexes.size() - 1);
  auto minStep = pow(10.0f, -digits);
  for (size_t i = 0; i < indexes.size() - 1; ++i)
    dim.push_back(DimensionStep(constraints[indexes[i]], vals[indexes[i]], minStep, steps));

  dim[0].reset(1);
}

bool InitialPointProvider::getNext()
{
  while (idx >= 0)
  {
    float remaining = dim[idx].bump();
    if (remaining >= 0)
    {
      ++idx;
      if (static_cast<size_t>(idx) < dim.size())
      {
        dim[idx].reset(remaining);
      }
      else
      {
        auto& constraint = constraints[indexes[idx]];
        auto& val = vals[indexes[idx]];
        --idx;
        if ((!constraint.required() && remaining == 0.f)
          || (remaining >= constraint.min() && remaining <= constraint.max()))
        {
          val = remaining;
          return true;
        }
      }
    }
    else
      --idx;
  }
  return false;
}

InitialPointProvider::DimensionStep::DimensionStep(const Constraint& c, float& val, float minStep, size_t maxStepCount)
  : constraint(c), val(val), minStep(minStep), maxStepCount(maxStepCount),
    stepWidth(c.width() > 0 ? std::max(c.width() / (maxStepCount - 1), minStep) : 0)
{
}

void InitialPointProvider::DimensionStep::reset(float newRemaining)
{
  remaining = newRemaining;
  step = 0;

  // first bump is skip, but only if it's not required and min > 0
  skipFirst = !constraint.required() && constraint.min() > 0;

  max = constraint.max() < remaining ? constraint.max() : remaining;
  auto span = max - constraint.min();

  if (span <= 0)
  {
    stepWidth = 0;
    stepCount = skipFirst ? 1 : 0;
    return;
  }

  size_t spansLeft = maxStepCount - 1;
  if (skipFirst)
    --spansLeft;
  if (spansLeft < 1)
    spansLeft = 1;

  stepWidth = constraint.width() / spansLeft;
  if (stepWidth < minStep)
    stepWidth = minStep;

  stepCount = static_cast<size_t>(ceil(span / stepWidth));
}

float InitialPointProvider::DimensionStep::bump()
{
  if (skipFirst)
  {
    skipFirst = false;
    val = 0;
    return remaining;
  }

  if (step > stepCount)
    return -1;

  val = step == stepCount ? max : round((constraint.min() + step*stepWidth) / minStep)*minStep;
  ++step;

  return remaining - val;
}
