#include "WavetableGenerator.h"

juce::Array<float> WavetableGenerator::createSineWavetable() {
  juce::Array<float> wavetable;  
  for (int i = 0; i < wtSize; i++) {
    wavetable.insert(i, std::sin(juce::MathConstants<double>::twoPi
				 * i/wtSize));
  }
  return wavetable;
}

juce::Array<float> WavetableGenerator::createSawtoothWavetable() {
  juce::Array<float> wavetable;
  for (int i = 0; i < wtSize; i++) {
    wavetable.insert(i, (juce::MathConstants<double>::twoPi * i / wtSize) /
                            juce::MathConstants<float>::pi);
  }
  return wavetable;
}

juce::Array<float> WavetableGenerator::createSquareWavetable() {
  juce::Array<float> wavetable;
  for (int i = 0; i < wtSize; i++) {
    if (i <= wtSize / 2) {
      wavetable.insert(i, 2);
    } else {
      wavetable.insert(i, 0);
    }
  }
  return wavetable;
}


