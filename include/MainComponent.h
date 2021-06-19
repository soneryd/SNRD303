#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "WavetableGenerator.h"

class MainComponent   : public juce::AudioAppComponent,
			public juce::Slider::Listener {
public:
  MainComponent();
  void paint (juce::Graphics&) override;
  void resized() override;
  void releaseResources() override {}
  void prepareToPlay(int, double sampleRate) override;
  void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
  void sliderValueChanged(juce::Slider* slider) override;
  
private:
  juce::Slider frequencySlider;
  juce::Array<float> wavetable;
  double frequency;
  double angleDelta;
  double sampleRate;
  double phase;
  double wtSize;  

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
