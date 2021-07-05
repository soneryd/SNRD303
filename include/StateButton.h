#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "SeqButton.h"

class StateButton : public juce::ToggleButton {
 public:
  void paint(juce::Graphics &g) {
    if (state == active) {
      g.fillAll(juce::Colour(220, 110, 10));
      g.setColour (juce::Colours::white);
      g.setFont (14.0f);
      g.drawText(text, getLocalBounds(),
		 juce::Justification::centred, true);      
    } else if (state == inactive) {
      g.fillAll(juce::Colour(20, 31, 51));
      g.setColour (juce::Colours::white);
      g.setFont (14.0f);
      g.drawText(text, getLocalBounds(),
		 juce::Justification::centred, true);      
    } else if (state == hold) {
      g.fillAll(juce::Colour(220, 110, 10));
      g.setColour (juce::Colours::white);
      g.setFont (14.0f);
      g.drawText("-", getLocalBounds(),
		 juce::Justification::centred, true);
    }
  }

  void setState(int state) { this->state = state;}
  int getState() { return state; };

  void setText(std::string text) {
    this->text = text;
  }

private:
  int state;
  std::string text="";
};
