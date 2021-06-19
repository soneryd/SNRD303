#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

class WavetableGenerator {
 private:
  static const int wtSize = 1024;
 public:
  static juce::Array<float> createSineWavetable();
  static juce::Array<float> createSawtoothWavetable();
  static juce::Array<float> createSquareWavetable();
};

