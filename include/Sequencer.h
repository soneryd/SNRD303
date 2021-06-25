#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "KeyboardButton.h"
#include "StateButton.h"

#define NUMSTEPS 8

struct SeqData {
  std::vector<int>* seqTrig;
  std::vector<float>* seqFreq;
  int* beatCount;
  double* freq;
  double* angleDelta;
  double sampleRate;
  int wtSize;
};

class Sequencer : public juce::Component,
		  public juce::KeyListener {
public:
  Sequencer();
  ~Sequencer();

  void paint(juce::Graphics &g);
  void resized();
  void setEnv(juce::ADSR *env) {this->env = env;};
  void setSeqData(std::vector<int>* seqTrig, std::vector<float>* seqFreq,
		  int* beatCount, double* freq, double* angleDelta,
		  double sampleRate, int wtSize);
  void setFrequency(double frequency);
  void toggleSeqButton(int beat, int state);
  void setRunning(bool running);
  void setRecording(bool recording);
  void mouseDown(const juce::MouseEvent &event);
  void mouseUp(const juce::MouseEvent &event);
  bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;

private:
  StateButton startButton;
  StateButton recButton;
  StateButton seqLights[NUMSTEPS];
  StateButton seqButtons[NUMSTEPS];
  KeyboardButton keyboardButtons[13];
  bool running;
  bool recording;
  struct SeqData seqData;
  juce::ADSR* env;
  std::vector<int>* seqTrig;
  std::vector<float>* seqFreq;
  int* beatCount;
  float seqFrequency[NUMSTEPS];
  double* frequency;
  double* angleDelta;
  double sampleRate;
  int wtSize;
};
