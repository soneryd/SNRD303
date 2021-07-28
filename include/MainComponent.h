#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "CustomLookAndFeel.h"
#include "CustomLabel.h"
#include "Sequencer.h"
#include "WavetableGenerator.h"
#include "SeqButton.h"

enum filterParams {
  cutoff,
  resonance,
  accent
};

enum adsrParams {
  atk,
  dec,
  sus,
  rel
};

class MainComponent : public juce::AudioAppComponent,
    private juce::Timer, private juce::Slider::Listener {
public:
  MainComponent();
  void paint (juce::Graphics&) override;
  void resized() override;
  void releaseResources() override {}
  void prepareToPlay(int samplesPerBlock, double sampleRate) override;
  void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
  void sequencerStep();
  void sliderValueChanged(juce::Slider* slider) override;
  double line(double startVal, double goalVal, double ms);
  void setFrequency(float freq);
  void setFilterCutOff(double cutOff);  
  void timerCallback() override;
  void mouseDown(const juce::MouseEvent &event) override;
  double doubleMax(double val1, double val2);

private:
  Sequencer sequencer;
  CustomLookAndFeel lookAndFeel;
  juce::Slider filterSliders[3];
  juce::Slider adsrSliders[4];
  CustomLabel filterLabels[3];  
  CustomLabel adsrLabels[4];  
  juce::Slider waveSlider;
  int buf[sizeof(int)];
  juce::AbstractFifo abstractFifo{1024};
  SeqButton startButton;
  SeqButton recButton;
  juce::TextEditor bpmBox;
  bool runningTimer;
  juce::Array<float> wavetable;
  juce::Array<float> sqrWavetable;
  juce::Array<float> subtable;
  juce::ADSR::Parameters envParameters;
  juce::ADSR* env;
  juce::dsp::StateVariableTPTFilter<float> filter;
  double waveRatio;
  int sampleCount;
  int* beatCount;
  int bpm;
  std::vector<int>* seqTrig;
  std::vector<float>* seqFreq;
  double cutOff;
  double accentLevel;
  double* frequency;
  double* angleDelta;
  double sampleRate;
  double phase;
  double subPhase;
  double wtSize;  

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
