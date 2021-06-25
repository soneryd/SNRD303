#include "Sequencer.h"

Sequencer::Sequencer() {
  addMouseListener(this, true);
  addKeyListener(this);

  // Sequencer buttons
  for (int i = 0; i < 8; i++) {
    addAndMakeVisible(seqButtons[i]);
    addAndMakeVisible(seqLights[i]);
  };
  // Keyboard buttons
  std::vector<int> whitekeys {0,2,4,5,7,9,11,12};
  std::vector<int> blackkeys { 1, 3, 6, 8, 10 };
  for (int i = 0; i < (int)whitekeys.size(); i++) {
    keyboardButtons[whitekeys.at(i)].setType(white);
    addAndMakeVisible(keyboardButtons[whitekeys.at(i)]);
  }

  for (int i = 0; i < (int)blackkeys.size(); i++) {
    keyboardButtons[blackkeys.at(i)].setType(black);
    addAndMakeVisible(keyboardButtons[blackkeys.at(i)]);    
  }
}

Sequencer::~Sequencer() {}

void Sequencer::paint(juce::Graphics &g) {
  //g.fillAll(juce::Colours::aqua);
}

void Sequencer::resized() {
  //Start/Rec buttons
  startButton.setBounds(650, 80, 60, 40);
  recButton.setBounds(650, 130, 60, 40);    
  
  // Sequencer
  for (int i = 0; i < 8; i++) {
    int height, width;
    height = width = 40;
    int y = 50;
    int x = (i*(getWidth()-60)/8)+(getWidth()/8/2)-(width/2);
    seqButtons[i].setBounds(x+30, y, width, height);

    y = 20;
    height = width = 10;
    x = (i*(getWidth()-60)/8)+(getWidth()/8/2)-(width/2);
    seqLights[i].setBounds(x+30, y, width, height);
  }

  // Keyboard
  std::vector<int> whitekeys {0,2,4,5,7,9,11,12};
  for (int i = 0; i < (int) whitekeys.size(); i++) {
    keyboardButtons[whitekeys.at(i)]
      .setBounds(75+((i+1)*50), 120, 50, 120);
  }
  std::vector<int> blackkeys { 1, 3, 6, 8, 10 };
  for (int i = 0; i < (int) blackkeys.size(); i++) {
    if (blackkeys.at(i) % 2 != 0) {
      keyboardButtons[blackkeys.at(i)]
	.setBounds(140 + (blackkeys.at(i)*(50/2)), 120, 25, 80);
    } else {
      keyboardButtons[blackkeys.at(i)]
	.setBounds(140 + ((blackkeys.at(i)+1)*(50/2)), 120, 25, 80);      
    }
  }
}

void Sequencer::setRunning(bool running){
  this->running = running;
}

void Sequencer::setRecording(bool recording) {this->recording = recording;}

void Sequencer::mouseDown(const juce::MouseEvent &event) {
  // Sequencer buttons
  for (int i = 0; i < 8; i++) {
    if (event.eventComponent == &seqButtons[i]) {
      seqTrig->at(i) = (seqTrig->at(i)+1)%3;
      seqButtons[i].setState(seqTrig->at(i));
      seqButtons[i].repaint();
    }
  }

  // Keyboard buttons
  for(int i = 0; i < 13; i++) {
    if (event.eventComponent == &keyboardButtons[i]) {
      float freq = juce::MidiMessage::getMidiNoteInHertz(i+69);      
      setFrequency(freq);
      env->noteOn();
      if (recording) {
        seqTrig->at(*beatCount%8) = 1;
        seqFreq->at(*beatCount%8) = freq;

        seqButtons[*beatCount%8].setState(active);
        seqButtons[*beatCount%8].repaint();

        if (!running) {
          seqLights[(*beatCount+1)%8].setState(active);
          seqLights[*beatCount%8].setState(inactive);
          seqLights[(*beatCount+1)%8].repaint();
	  seqLights[*beatCount%8].repaint();
          *beatCount = *beatCount+1;
        }
      }

      keyboardButtons[i].setState(down);
      keyboardButtons[i].repaint();
    }
  }
}

void Sequencer::mouseUp(const juce::MouseEvent &event) {
  for(int i = 0; i < 13; i++) {
    if (event.eventComponent == &keyboardButtons[i]) {
      env->noteOff();
      keyboardButtons[i].setState(up);
      keyboardButtons[i].repaint();
    }
  }
}

bool Sequencer::keyPressed(const juce::KeyPress &key, juce::Component *originatingComponent) {
  if (key.getKeyCode() == 32 && !running && recording) {
    seqLights[(*beatCount+1)%8].setState(active);
    seqLights[*beatCount%8].setState(inactive);
    seqLights[(*beatCount+1)%8].repaint();
    seqLights[*beatCount%8].repaint();    
    *beatCount = *beatCount+1;
  }
}

void Sequencer::setFrequency(double frequency) {
  *angleDelta = frequency * wtSize / sampleRate;
}

void Sequencer::setSeqData(std::vector<int> *seqTrig, std::vector<float>* seqFreq,int *beatCount, double *freq,
                double *angleDelta, double sampleRate, int wtSize) {
  this->seqTrig = seqTrig;
  this->seqFreq = seqFreq;
  this->beatCount = beatCount;
  this->frequency = freq;
  this->angleDelta = angleDelta;
  this->sampleRate = sampleRate;
  this->wtSize = wtSize;
}

void Sequencer::toggleSeqButton(int beat, int state) {
  seqLights[beat].setState(state);
  seqLights[beat].repaint();
}
