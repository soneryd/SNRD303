#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "WavetableGenerator.h"
#include "SeqButton.h"

class MainComponent   : public juce::AudioAppComponent,
			public juce::Slider::Listener,
			private juce::Timer {
public:
  MainComponent();
  void paint (juce::Graphics&) override;
  void resized() override;
  void releaseResources() override {}
  void prepareToPlay(int, double sampleRate) override;
  void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
  void sliderValueChanged(juce::Slider* slider) override;
  void timerCallback() override;
  void buttonClicked(SeqButton* button, int state);

private:
  int buf[sizeof(int)];
  juce::AbstractFifo abstractFifo{1024};  
  SeqButton seqButtons[8];
  SeqButton seqPad[8];
  juce::Slider frequencySlider;
  juce::Array<float> wavetable;
  juce::ADSR env;
  int sampleCount;
  int* beatCount;
  int bpm;
  int seqData[8];
  double frequency;
  double angleDelta;
  double sampleRate;
  double phase;
  double wtSize;  

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
