#pragma once

#include "Constraint.h"

class InitialPointProvider
{
  const std::vector<Constraint>& constraints;
  std::vector<float>& vals;

  std::vector<size_t> indexes;
  class DimensionStep;
  std::vector<DimensionStep> dim;
  int idx;

public:
  InitialPointProvider(const std::vector<Constraint>& constraints, std::vector<float>& vals, int digits, size_t steps);

  bool getNext();

private:

  class DimensionStep
  {
    const Constraint &constraint;
    float& val;

    float minStep;
    size_t maxStepCount;

    size_t stepCount;
    size_t step;
    float max;
    bool skipFirst;
    float stepWidth;
    float remaining;

  public:
    DimensionStep(const Constraint& c, float& val, float minStep, size_t maxStepCount);

    void reset(float remaining);
    float bump();
  };

};

