#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
  setAudioChannels (0, 2);
  // GUI
  frequencySlider.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
  addAndMakeVisible(frequencySlider);
  frequencySlider.setRange(0, 1400);
  frequencySlider.setValue (440.0);  
  frequencySlider.addListener(this);
  setSize(600, 400);
  
  // Audio parameters
  this->wavetable = WavetableGenerator::createSawtoothWavetable();
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
  auto level = 0.125f;
  auto* lSpeaker  = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
  auto* rSpeaker = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);

  for (auto sample = 0; sample < bufferToFill.numSamples; ++sample){
    lSpeaker[sample] = wavetable[(int)phase] * level*0.2;
    rSpeaker[sample] = wavetable[(int)phase] * level*0.2;
    phase = fmod((phase + angleDelta), wtSize);
  }
}
