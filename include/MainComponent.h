#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "WavetableGenerator.h"
#include "SeqButton.h"
#include "KeyboardButton.h"

class MainComponent   : public juce::AudioAppComponent,
			private juce::Timer {
public:
  MainComponent();
  void paint (juce::Graphics&) override;
  void resized() override;
  void releaseResources() override {}
  void prepareToPlay(int, double sampleRate) override;
  void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
  void setFrequency(float freq);
  void timerCallback() override;
  void buttonClicked(SeqButton* button, int state);
  void mouseDown(const juce::MouseEvent &event) override;
  void mouseUp(const juce::MouseEvent &event) override;

private:
  int buf[sizeof(int)];
  juce::AbstractFifo abstractFifo{1024};
  KeyboardButton keyboardButtons[13];
  SeqButton startButton;
  SeqButton recButton;
  bool runningTimer;
  SeqButton seqButtons[8];
  SeqButton seqPad[8];
  juce::Array<float> wavetable;
  juce::ADSR env;
  int sampleCount;
  int* beatCount;
  int bpm;
  int seqTriggers[8];
  float seqFrequency[8];
  double frequency;
  double angleDelta;
  double sampleRate;
  double phase;
  double wtSize;  

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
