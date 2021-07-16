#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class CustomLabel : public juce::Label {
public:
  void componentMovedOrResized(Component &component, bool wasMoved,
                               bool wasResized) override{
    this->setColour(juce::Label::textColourId, juce::Colours::black);
    this->setJustificationType(juce::Justification::centred);
    const juce::Font f(getLookAndFeel().getLabelFont (*this));
    setSize(component.getWidth(), juce::roundToInt(f.getHeight()));
    setTopLeftPosition (component.getX(), component.getY() - getHeight());    
  };
};
