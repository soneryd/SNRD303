#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

enum type {black, white};
enum keyboardState {up, down};

class KeyboardButton : public juce::ToggleButton {
public:
  void paint(juce::Graphics &g) {
    if(type==white && state==up) {
      g.fillAll(juce::Colours::white);
      g.setColour(juce::Colours::antiquewhite);
      g.fillRect(0, 0, 2, getHeight());
    } else if (type==black && state==up) {
      g.fillAll(juce::Colours::black);      
    } else if (state == down) {
      g.fillAll(juce::Colours::aquamarine);
    }
  }

  void setType(int type) {
    this->type = type;
  }
  
  void setState(int state) {
    this->state = state;
  }

private:
  int type, state;
};
