#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() {
  setAudioChannels (0, 2);  
  sampleCount = 0;
  beatCount = new int(0);
  for (int i = 0; i < 8; i++) {
    seqTriggers[i] = 0;
    seqFrequency[i] = 440;
  }
  bpm = 90;
  // GUI
  addMouseListener(this, true);
  addAndMakeVisible(startButton);
  addAndMakeVisible(recButton);
  startButton.setText("START");
  recButton.setText("REC");
  runningTimer=false;

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
  
  // Sequencer buttons
  for (int i = 0; i < 8; i++) {
    addAndMakeVisible(seqButtons[i]);
    addAndMakeVisible(seqPad[i]);
    seqPad[i].onClick = [this, i] {
      if (seqTriggers[(i+7)%8] == inactive) {
        seqTriggers[i] = (seqTriggers[i] + 1) % 2;
      } else {
        seqTriggers[i] = (seqTriggers[i] + 1) % 3;	
      }
      buttonClicked(&seqPad[i], seqTriggers[i]);
    };
  };

  // Audio parameters
  this->wavetable = WavetableGenerator::createSawtoothWavetable();
  juce::ADSR::Parameters params = {0.001, 0.05, 0.7, 0.1};
  this->env.setParameters(params);
}

void MainComponent::paint (juce::Graphics& g) {
  g.fillAll(juce::Colours::antiquewhite);
}

void MainComponent::resized() {
  // Start/Stop
  startButton.setBounds(600, 80, 60, 40);
  recButton.setBounds(600, 130, 60, 40);  
  
  // Sequencer
  for (int i = 0; i < 8; i++) {
    seqButtons[i].setBounds(65+((i+1)*60), 20, 10, 10);
    seqPad[i].setBounds(50+((i+1)*60), 40, 40, 40);
  }

  // Keyboard
  std::vector<int> whitekeys {0,2,4,5,7,9,11,12};
  for (int i = 0; i < (int) whitekeys.size(); i++) {
    keyboardButtons[whitekeys.at(i)]
      .setBounds(90+((i+1)*50), 100, 50, 120);
  }
  std::vector<int> blackkeys { 1, 3, 6, 8, 10 };
  for (int i = 0; i < (int) blackkeys.size(); i++) {
    if (blackkeys.at(i) % 2 != 0) {
      keyboardButtons[blackkeys.at(i)]
	.setBounds(155 + (blackkeys.at(i)*(50/2)), 100, 25, 80);
    } else {
      keyboardButtons[blackkeys.at(i)]
	.setBounds(155 + ((blackkeys.at(i)+1)*(50/2)), 100, 25, 80);      
    }
  }
}

void MainComponent::setFrequency(float freq) {
  angleDelta = freq * wtSize / sampleRate;      
}

void MainComponent::prepareToPlay(int, double sampleRate){
  this->sampleRate = sampleRate;
  frequency = 440;
  phase = 0;
  wtSize = 1024;
  angleDelta = frequency * wtSize / sampleRate;
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) {
  float level, envLevel;
  auto* lSpeaker  = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
  auto* rSpeaker = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);
  
  for (auto sample = 0; sample < bufferToFill.numSamples; ++sample){
    if(runningTimer) {
      sampleCount++;
      if (sampleCount == (int)((sampleRate*60)/(bpm*6))) {
	if (seqTriggers[(*beatCount+1)%8] != hold) {
	  env.noteOff();
	}      
      }

      if (sampleCount == (int) ((sampleRate*60)/(bpm*4))) {
	sampleCount = 0;
	*beatCount = *beatCount+1;

	if (seqTriggers[*beatCount%8] == active) {
	  setFrequency(seqFrequency[*beatCount%8]);
	  env.noteOn();
	}

	int start1, size1, start2, size2;
	abstractFifo.prepareToWrite(1, start1, size1, start2, size2);
	if (size1 > 0) {
	  memcpy(buf, beatCount, sizeof(int));
	}
	abstractFifo.finishedWrite(size1 + size2);
      }
    }

    envLevel = env.getNextSample();
    level = 0.125f * envLevel;    
    
    lSpeaker[sample] = wavetable[(int)phase] * level*0.2;
    rSpeaker[sample] = wavetable[(int)phase] * level*0.2;
    phase = fmod((phase + angleDelta), wtSize);
  }
}

void MainComponent::timerCallback() {
  int start1, size1, start2, size2;
  abstractFifo.prepareToRead (1, start1, size1, start2, size2);

  if (size1 > 0) {
    seqButtons[*buf%8].setState(active);
    seqButtons[(*buf+7)%8].setState(inactive);
    seqButtons[*buf%8].repaint();
    seqButtons[(*buf%8+7)%8].repaint();    
  }
  abstractFifo.finishedRead (size1 + size2);
  
}

void MainComponent::buttonClicked(SeqButton* button, int state) {
  button->setState(state);
  button->repaint();
}

void MainComponent::mouseDown(const juce::MouseEvent &event) {
  for(int i = 0; i < 13; i++) {
    if (event.eventComponent == &keyboardButtons[i]) {
      env.noteOn();
      float freq = juce::MidiMessage::getMidiNoteInHertz(i+69);
      setFrequency(freq);
      if (recButton.getState() == active) {
	seqTriggers[*beatCount%8] = 1;
	seqFrequency[*beatCount%8] = freq;
	seqPad[*beatCount%8].setState(active);
	seqPad[*beatCount%8].repaint();
        if (!runningTimer) {
	  seqButtons[*beatCount%8].setState(active);
	  seqButtons[*beatCount+7%8].setState(active);
	  seqButtons[*beatCount%8].repaint();
	  seqButtons[*beatCount+7%8].repaint();	  
          *beatCount = *beatCount + 1;
        }
      }
      keyboardButtons[i].setState(down);
      keyboardButtons[i].repaint();
    } else if (event.eventComponent == &startButton) {
      if (runningTimer) {
	seqButtons[*beatCount%8].setState(inactive);
	seqButtons[*beatCount%8].repaint();
	*beatCount=0;
	sampleCount = 0;		
        startButton.setState(inactive);
	startButton.setText("START");
	env.noteOff();
        Timer::stopTimer();
        runningTimer = false;
      } else {
        startButton.setState(active);
	startButton.setText("STOP");
        Timer::startTimer(1);
        runningTimer = true;	
      }
    } else if (event.eventComponent == &recButton) {
      recButton.getState() == inactive ? recButton.setState(active)
                                       : recButton.setState(inactive);
    }
  }
}

void MainComponent::mouseUp(const juce::MouseEvent &event) {
  for(int i = 0; i < 13; i++) {
    if (event.eventComponent == &keyboardButtons[i]) {
      env.noteOff();
      keyboardButtons[i].setState(up);
      keyboardButtons[i].repaint();
    }
  }
}
