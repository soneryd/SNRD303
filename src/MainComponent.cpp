#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() {
  std::cout << "constructor\n";
  setSize(800, 600);
  setAudioChannels (0, 2);  
  sampleCount = 0;
  addAndMakeVisible(sequencer);
  bpm = 90;

  // GUI
  addMouseListener(this, true);
  addAndMakeVisible(startButton);
  addAndMakeVisible(recButton);
  startButton.setText("START");
  recButton.setText("REC");
  runningTimer=false;

  // Audio parameters
  this->wavetable = WavetableGenerator::createSawtoothWavetable();
  juce::ADSR::Parameters params = {0.001, 0.05, 0.7, 0.1};

  this->env->setParameters(params);
}

void MainComponent::paint (juce::Graphics& g) {
  g.fillAll(juce::Colours::antiquewhite);
}

void MainComponent::resized() {
  // Sequencer
  sequencer.setBounds(50, getHeight()-320, getWidth()-100, 300);

  // Start/Stop
  startButton.setBounds(700, 400, 60, 40);
  recButton.setBounds(700, 450, 60, 40);  
}

void MainComponent::setFrequency(float freq) {
  *angleDelta = freq * wtSize / sampleRate;      
}

void MainComponent::prepareToPlay(int, double sampleRate){
  this->env = new juce::ADSR;  
  this->seqTrig = new std::vector<int>;
  this->seqFreq = new std::vector<float>;    
  this->sampleRate = sampleRate;
  frequency = new double(440.0);
  angleDelta = new double;
  beatCount = new int(0);  
  phase = 0;
  wtSize = 1024;

  for (int i = 0; i < NUMSTEPS; i++) {
    seqTrig->push_back(0);
    seqFreq->push_back(440);
  }
  
  sequencer.setSeqData(seqTrig, seqFreq, beatCount, frequency,
		       angleDelta, sampleRate, wtSize);

  sequencer.setEnv(env);
  setFrequency(*frequency);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) {
  float level, envLevel;
  auto* lSpeaker=bufferToFill.buffer->getWritePointer(0,bufferToFill.startSample);
  auto* rSpeaker=bufferToFill.buffer->getWritePointer(1,bufferToFill.startSample);
  
  for (auto sample = 0; sample < bufferToFill.numSamples; ++sample){
    if(runningTimer) {
      sampleCount++;
      if (sampleCount == (int)((sampleRate*60)/(bpm*6))) {
	if (seqTrig->at((*beatCount+1)%8) != hold) {
	  env->noteOff();
	}      
      }

      if (sampleCount == (int) ((sampleRate*60)/(bpm*4))) {
	sampleCount = 0;
	*beatCount = *beatCount+1;

	if (seqTrig->at(*beatCount%8) == active) {
	  setFrequency(seqFreq->at(*beatCount%8));
	  env->noteOn();
	}

	int start1, size1, start2, size2;
	abstractFifo.prepareToWrite(1, start1, size1, start2, size2);
        if (size1 > 0) {
          memcpy(buf, beatCount, sizeof(int));
        }
        abstractFifo.finishedWrite(size1 + size2);
      }
    }

    envLevel = env->getNextSample();
    level = 0.125f * envLevel;    
    
    lSpeaker[sample] = wavetable[(int)phase] * level*0.2;
    rSpeaker[sample] = wavetable[(int)phase] * level*0.2;
    phase = fmod((phase + *angleDelta), wtSize);
  }
}

void MainComponent::timerCallback() {
  int start1, size1, start2, size2;
  abstractFifo.prepareToRead (1, start1, size1, start2, size2);
  if (size1 > 0) {
    sequencer.toggleSeqButton(*buf%8, active);
    sequencer.toggleSeqButton((*buf+7)%8, inactive);    
  }
  abstractFifo.finishedRead (size1 + size2);
}

void MainComponent::mouseDown(const juce::MouseEvent &event) {
  for(int i = 0; i < 13; i++) {
    if (event.eventComponent == &startButton) {
      if (runningTimer) {
        runningTimer = false;	
        Timer::stopTimer();
	sequencer.toggleSeqButton(*beatCount%8, inactive);
	sequencer.setRunning(runningTimer);	
	*beatCount=0;
	sampleCount = 0;		
        startButton.setState(inactive);
	startButton.setText("START");
	env->noteOff();
      } else {
        Timer::startTimer(1);	
        startButton.setState(active);
	startButton.setText("STOP");
        runningTimer = true;
	sequencer.setRunning(runningTimer);	
      }
    } else if (event.eventComponent == &recButton) {
      if (recButton.getState() == inactive) {
	recButton.setState(active);
	sequencer.setRecording(true);
	sequencer.toggleSeqButton(*beatCount%8, active);
      } else {
	recButton.setState(inactive);
	sequencer.setRecording(false);
	sequencer.toggleSeqButton(*beatCount%8, inactive);
	*beatCount=0;
      }
    }
  }
}
