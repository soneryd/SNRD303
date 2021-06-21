#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

enum buttonStates{inactive, active, hold, slide};

class SeqButton : public juce::ToggleButton {
 public:
  void paint(juce::Graphics &g) {
    if (state == active) {
      g.fillAll(juce::Colour(223, 116, 12));
    } else if (state == inactive) {
      g.fillAll(juce::Colour(20, 31, 51));
    } else if (state == hold) {
    }
  }

  void setState(int state) {
    this->state = state;
  }

private:
  int state;
};
