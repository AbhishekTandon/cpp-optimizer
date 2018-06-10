#pragma once

class Constraint
{
  bool _required;
  float _min;
  float _max;

public:
  Constraint(bool required, float min, float max) : _required(required), _min(min), _max(max)
  {
    if (_min < 0.0f)
      throw std::runtime_error("min < 0");
    if (_max < _min)
      throw std::runtime_error("max < min");
    if (_max > 1.0f)
      throw std::runtime_error("max > 1");
  }

  bool required() const { return _required; }
  float min() const { return _min; }
  float max() const { return _max; }
  float width() const { return _max - _min; }
};
