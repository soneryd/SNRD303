#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "Sequencer.h"
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
  void mouseDown(const juce::MouseEvent &event) override;

private:
  Sequencer sequencer;
  int buf[sizeof(int)];
  juce::AbstractFifo abstractFifo{1024};
  SeqButton startButton;
  SeqButton recButton;
  bool runningTimer;
  juce::Array<float> wavetable;
  juce::ADSR* env;
  int sampleCount;
  int* beatCount;
  int bpm;
  std::vector<int>* seqTrig;
  std::vector<float>* seqFreq;
  double* frequency;
  double* angleDelta;
  double sampleRate;
  double phase;
  double wtSize;  

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
