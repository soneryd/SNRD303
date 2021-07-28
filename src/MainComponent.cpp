#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() {
  setSize(800, 600);
  setBounds(0, 0, 800, 600);
  setAudioChannels (0, 2);  
  sampleCount = 0;
  bpm = 120;    
  addAndMakeVisible(sequencer);
  cutOff = 440;
  accentLevel = 0.01;

  // BPM
  bpmBox.setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(3, "1234567890"), true);
  bpmBox.setText("120", false);
  addAndMakeVisible(bpmBox);
  bpmBox.onReturnKey = [this]() {
    bpm = std::stoi(bpmBox.getText().toStdString());
  };

  waveSlider.setLookAndFeel(&lookAndFeel);
  waveSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
  waveSlider.setRange(0, 100, 0.5);
  waveSlider.setValue(50);
  waveRatio = 50;
  waveSlider.onValueChange = [this]() {
    waveRatio = waveSlider.getValue();
    std::cout << waveRatio/100 << ":" << ((100-waveRatio)/100) << std::endl;
  };

  addAndMakeVisible(waveSlider);

  // Filter sliders
  for (int i = 0; i < 3; i++) {
    filterSliders[i].setLookAndFeel(&lookAndFeel);
    addAndMakeVisible(filterSliders[i]);
    addAndMakeVisible(filterLabels[i]);
    filterLabels[i].attachToComponent(&filterSliders[i], true);
    filterSliders[i].setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    filterSliders[i].setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    filterSliders[i].setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::antiquewhite);
    //filterSliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
    filterSliders[i].setTextBoxStyle(juce::Slider::NoTextBox, true, 0,0);
    filterSliders[i].setRange(1, 10, 0.01);
    filterSliders[i].addListener(this);
  }
  filterLabels[cutoff].setText ("CutOff", juce::dontSendNotification);
  filterLabels[resonance].setText ("Resonance", juce::dontSendNotification);
  filterLabels[accent].setText ("Accent", juce::dontSendNotification);
  
  for (int i = 0; i < 4; i++) {
    adsrSliders[i].setLookAndFeel(&lookAndFeel);    
    addAndMakeVisible(adsrSliders[i]);
    addAndMakeVisible(adsrLabels[i]);
    adsrLabels[i].attachToComponent(&adsrSliders[i], true);    
    adsrSliders[i].setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    adsrSliders[i].setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    adsrSliders[i].setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::antiquewhite);
    //adsrSliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 20);
    adsrSliders[i].setTextBoxStyle(juce::Slider::NoTextBox, true, 0,0);    
    adsrSliders[i].setRange(0, 1, 0.01);
    adsrSliders[i].addListener(this);    
  }

  adsrLabels[atk].setText("Attack", juce::dontSendNotification);
  adsrLabels[dec].setText("Decay", juce::dontSendNotification);
  adsrLabels[sus].setText("Sustain", juce::dontSendNotification);
  adsrLabels[rel].setText("Release", juce::dontSendNotification);

  adsrSliders[atk].setValue(0.01);
  adsrSliders[dec].setValue(0.1);
  adsrSliders[sus].setValue(0.7);
  adsrSliders[rel].setValue(0.01);

  // GUI
  addMouseListener(this, true);
  addAndMakeVisible(startButton);
  addAndMakeVisible(recButton);
  startButton.setText("START");
  recButton.setText("REC");
  runningTimer=false;

  // Audio parameters
  this->wavetable = WavetableGenerator::createSawtoothWavetable();
  this->sqrWavetable = WavetableGenerator::createSquareWavetable();
  this->subtable  = WavetableGenerator::createSineWavetable();
  envParameters = {0.001, 0.01, 0.8, 0.1};
  this->env->setParameters(envParameters);
}

void MainComponent::paint (juce::Graphics& g) {
  g.fillAll(juce::Colours::antiquewhite);
}

void MainComponent::resized() {
  // Sequencer
  sequencer.setBounds(50, getHeight()-320, getWidth()-100, 300);

  waveSlider.setBounds(550, 120, 30, 130);
  waveSlider.setColour(juce::Slider::textBoxOutlineColourId, 
		       juce::Colours::transparentBlack);

  // Filter sliders
  for (int i = 0; i < 3; i++) {
    filterSliders[i].setBounds(50+(i*70), 50, 60, 60);
    filterSliders[i].setColour(juce::Slider::textBoxOutlineColourId, 
			       juce::Colours::transparentBlack);
  }

  for (int i = 0; i < 4; i++) {
    adsrSliders[i].setBounds(300+(i*70), 50, 60, 60);
    adsrSliders[i].setColour(juce::Slider::textBoxOutlineColourId, 
			       juce::Colours::transparentBlack);
  }

  // Start/Stop
  startButton.setBounds(700, 400, 60, 40);
  recButton.setBounds(700, 450, 60, 40);
  bpmBox.setBounds(700, 500, 60, 40);
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
    setFilterCutOff(cutOff * doubleMax(envLevel, accentLevel));

    double sampleVal = wavetable[(int)phase] * (waveRatio/100);
    sampleVal += sqrWavetable[(int)phase] * ((100-waveRatio)/100);
    sampleVal += subtable[(int)subPhase];

    lSpeaker[sample] = sampleVal * level;
    rSpeaker[sample] = sampleVal * level;
    phase = fmod((phase + *angleDelta), wtSize);
    subPhase = fmod((subPhase + (*angleDelta/2)), wtSize);
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
	if (seqTrig->at((*beatCount+1)%NUMSTEPS) != hold &&
	    seqTrig->at((*beatCount+1)%NUMSTEPS) != slide) {
	  env->noteOff();
	}      
      }

      if (seqTrig->at((*beatCount) % NUMSTEPS) == slide) {
	*frequency = *frequency+line(*frequency,
				     seqFreq->at((*beatCount) % NUMSTEPS),
				     20);
	setFrequency(*frequency);
      }

      if (sampleCount == (int) ((sampleRate*60)/(bpm*4))) {
	sampleCount = 0;
	*beatCount = *beatCount+1;

	if (seqTrig->at(*beatCount%NUMSTEPS) == active) {
	  *frequency = seqFreq->at(*beatCount%NUMSTEPS);
	  setFrequency(*frequency);
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
    //cutOff = *frequency * filterSliders[cutoff].getValue();
    //cutOff = *frequency*(filterSliders[cutoff].getValue()-0.5);
    cutOff = (filterSliders[cutoff].getValue()-0.9)*1500;
    filter.setCutoffFrequency(cutOff);;
  } else if (slider == &filterSliders[resonance]) {
    filter.setResonance(filterSliders[resonance].getValue()+0.1);
  } else if (slider == &filterSliders[accent]) {
    accentLevel = 1-(filterSliders[accent].getValue())/10;
  } else if (slider == &adsrSliders[atk]) {
    envParameters.attack = adsrSliders[atk].getValue();
    env->setParameters(envParameters);
  } else if (slider == &adsrSliders[dec]) {
    envParameters.decay = adsrSliders[dec].getValue();
    env->setParameters(envParameters);    
  } else if (slider == &adsrSliders[sus]) {
    envParameters.sustain = adsrSliders[sus].getValue();
    env->setParameters(envParameters);    
  } else if (slider == &adsrSliders[rel]) {
    envParameters.release = adsrSliders[rel].getValue();
    env->setParameters(envParameters);    
  }
}

double MainComponent::line(double startVal, double goalVal, double ms) {
  double diff = (startVal-goalVal)*-1;
  double inc = diff/(sampleRate*(ms/1000));
  return inc;
}

void MainComponent::setFilterCutOff(double cutOff) {
  filter.setCutoffFrequency(cutOff);
}

void MainComponent::timerCallback() {
  int start1, size1, start2, size2;
  abstractFifo.prepareToRead (1, start1, size1, start2, size2);
  if (size1 > 0) {
    sequencer.toggleSeqButton(*buf%NUMSTEPS, active);
    sequencer.toggleSeqButton((*buf+(NUMSTEPS-1))%NUMSTEPS, inactive);    
  }
  abstractFifo.finishedRead (size1 + size2);
}

void MainComponent::mouseDown(const juce::MouseEvent &event) {
  for(int i = 0; i < 13; i++) {
    if (event.eventComponent == &startButton) {
      if (runningTimer) {
        runningTimer = false;	
        Timer::stopTimer();
	sequencer.toggleSeqButton(*beatCount%NUMSTEPS, inactive);
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
	sequencer.toggleSeqButton(*beatCount%NUMSTEPS, active);
      } else {
	recButton.setState(inactive);
	sequencer.setRecording(false);
	sequencer.toggleSeqButton(*beatCount%NUMSTEPS, inactive);
	*beatCount=0;
      }
    }
  }
}
