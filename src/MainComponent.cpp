#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() {
  setSize(800, 600);
  setAudioChannels (0, 2);  
  sampleCount = 0;
  bpm = 90;    
  addAndMakeVisible(sequencer);

  // Filter sliders
  for (int i = 0; i < 3; i++) {
    addAndMakeVisible(filterSliders[i]);
    filterSliders[i].setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    filterSliders[i].setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    filterSliders[i].setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::antiquewhite);
    filterSliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
    filterSliders[i].setRange(1, 10, 0.01);
    filterSliders[i].addListener(this);
  }
  
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

  // Filter sliders
  for (int i = 0; i < 3; i++) {
    filterSliders[i].setBounds(50+(i*70), 50, 80, 100);
  }

  // Start/Stop
  startButton.setBounds(700, 400, 60, 40);
  recButton.setBounds(700, 450, 60, 40);  
}

void MainComponent::setFrequency(float freq) {
  *angleDelta = freq * wtSize / sampleRate;      
}

void MainComponent::prepareToPlay(int samplesPerBlock, double sampleRate){
  juce::dsp::ProcessSpec spec;
  spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
  spec.sampleRate = sampleRate;
  spec.numChannels = 2;
  filter.prepare(spec);
  filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);

  
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
    sequencerStep();
    envLevel = env->getNextSample();
    level = 0.125f * envLevel;
    //setFilterCutOff(cutOff * doubleMax(envLevel, accent));
    setFilterCutOff(cutOff * envLevel);    
    double sampleVal = wavetable[(int)phase];
    lSpeaker[sample] = sampleVal * level;
    rSpeaker[sample] = sampleVal * level;
    phase = fmod((phase + *angleDelta), wtSize);
  }
  auto audioBlock = juce::dsp::AudioBlock<float>(*bufferToFill.buffer);
  auto context = juce::dsp::ProcessContextReplacing<float> (audioBlock);
  filter.process(context);  
}

double MainComponent::doubleMax(double val1, double val2) {
  if(val1 > val2)
    return val1;
  else
    return val2;
}

void MainComponent::sequencerStep() {
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
}

void MainComponent::sliderValueChanged(juce::Slider *slider) {
  if (slider == &filterSliders[cutoff]) {
    cutOff = *frequency * filterSliders[cutoff].getValue();
    filter.setCutoffFrequency(cutOff);;
  } else if (slider == &filterSliders[resonance]) {
    filter.setResonance(filterSliders[resonance].getValue()+0.1);
  } else if (slider == &filterSliders[accent]) {
    accentLevel = (filterSliders[accent].getValue())/10;
    std::cout << accentLevel << std::endl;
  }
}

void MainComponent::setFilterCutOff(double cutOff) {
  filter.setCutoffFrequency(cutOff);
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
