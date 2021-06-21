#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
  setAudioChannels (0, 2);  
  Timer::startTimer(1);
  sampleCount = 0;
  beatCount = new int(0);
  for (int i = 0; i < 8; i++) {
    seqData[i] = 0;
  }
  bpm = 90;
  // GUI

  // Button
  for (int i = 0; i < 8; i++) {
    seqButtons[i].Button::setToggleState(false, juce::dontSendNotification);
    seqPad[i].Button::setToggleState(false, juce::dontSendNotification);
    addAndMakeVisible(seqButtons[i]);
    addAndMakeVisible(seqPad[i]);
    seqPad[i].onClick = [this, i]{
      seqData[i] = (seqData[i]+1)%2;
      buttonClicked(&seqPad[i], seqData[i]);
    };
  };

  // Frequency slider
  frequencySlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
  addAndMakeVisible(frequencySlider);
  frequencySlider.setRange(0, 1400);
  frequencySlider.setValue (440.0);  
  frequencySlider.addListener(this);
  setSize(600, 400);
  
  // Audio parameters
  this->wavetable = WavetableGenerator::createSawtoothWavetable();
  juce::ADSR::Parameters params = {0.001, 0.05, 0.7, 0.1};
  this->env.setParameters(params);
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
  //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
  g.fillAll(juce::Colours::antiquewhite);
  //g.setFont (juce::Font (16.0f));
  //g.setColour (juce::Colours::white);
  //g.drawText ("Hello World!", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
  frequencySlider.setBounds(20, 20, 20, getHeight()-80);

  for (int i = 0; i < 8; i++) {
    seqButtons[i].setBounds(65+((i+1)*60), 20, 10, 10);
    seqPad[i].setBounds(50+((i+1)*60), 40, 40, 40);
  }
}

void MainComponent::sliderValueChanged (juce::Slider* slider) {
  if (slider == &frequencySlider) {
    frequency = frequencySlider.getValue();
    angleDelta = frequency * wtSize / sampleRate;    
  } 
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
    sampleCount++;

    if (sampleCount == (int)((sampleRate * 60)/(bpm*6))) {

      if (seqData[*beatCount%8] == active) {
	env.noteOff();
      }      
    }

    if (sampleCount == (int) ((sampleRate*60)/(bpm*4))) {
      sampleCount = 0;
      *beatCount = *beatCount+1;

      if (seqData[*beatCount%8] == active) {
        env.noteOn();
      }

      int start1, size1, start2, size2;
      abstractFifo.prepareToWrite(1, start1, size1, start2, size2);
      if(size1 > 0) {
	memcpy(buf, beatCount, sizeof(int));
      }
      abstractFifo.finishedWrite(size1+size2);
    }

    envLevel = env.getNextSample();
    level = 0.125f * 0.4 * envLevel;    
    
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

