#pragma once

#include <list>

struct ModulatedType
{
    // TODO
    T min_val;
    T max_val;
    T orig_val;
    T current_val;
};

class Modulator
{
  public:
    virtual ModulatedType snap(ModulatedType current) = 0;
};

// Thread safety notes: should ensure only access from 1 thread at once.
// tick() does updates, everything else is just a read.
class Parameter
{
  public:
    Parameter();

    const Parameter *modulate_with(Modulator mod);
    const Parameter *parent();

    void tick(); // Called on block update

    ModulatedType value;

  private:
    explicit Parameter(Parameter *parent, Modulator modulator);

    Modulator modulator_;
    std::list<Parameter> children_;
    const Parameter *parent_;
};

Parameter::Parameter() : parent_(nullptr) {}

Parameter::Parameter(Parameter *parent, Modulator modulator)
    : parent_(parent), modulator_(std::move(modulator))
{
}

const Parameter *Parameter::modulate_with(Modulator mod)
{
    children_.emplace_back(this, std::move(mod));
    return &children_.back();
}

const Parameter *Parameter::parent() { return parent_; }

void Parameter::tick()
{
    value = modulator_.snap(value);
    for (Parameter &p : children_)
    {
        // might need to integrate it somehow rather than pure destructively update, need to look at
        // what the current algorithm in surge is doing.
        p.value.current_val = value.current_val;
        p.tick();
    }
}
