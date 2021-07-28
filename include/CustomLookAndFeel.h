#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4 {
  void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height,
                        float sliderPosProportional, float rotaryStartAngle,
                        float rotaryEndAngle, juce::Slider &) override {
    float diameter = juce::jmin(width, height);
    float radius = diameter / 2;
    float centreX = x + width / 2;
    float centreY = y + height / 2;
    float rx = centreX - radius;
    float ry = centreY - radius;
    float angle = rotaryStartAngle +
                  (sliderPosProportional * (rotaryEndAngle - rotaryStartAngle));

    juce::Rectangle<float> dialArea(rx, ry, diameter, diameter);
    juce::Rectangle<float> dialArea2(rx, ry, diameter, diameter);

    juce::Path dialTick, backgndTick;
    dialTick.addPieSegment(dialArea, rotaryStartAngle, angle, 0.6);
    backgndTick.addPieSegment(dialArea, rotaryStartAngle, rotaryEndAngle, 0.6);

    g.setColour(juce::Colour(20, 31, 51));
    g.fillPath(backgndTick);

    g.setColour(juce::Colour(223, 116, 12));
    g.fillPath(dialTick);
  }

  void drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height,
                        float sliderPos, float minSliderPos, float maxSliderPos,
                        const juce::Slider::SliderStyle, juce::Slider &) override {
    
    juce::Rectangle<float> outerBodyRect(0, 0, width, height);
    juce::Rectangle<float> innerBodyRect(2, 2, width-4, height-4);
    juce::Rectangle<float> dialRect(0, sliderPos - (float) x, width, 6);
    juce::Path outerBody, innerBody, dial;
    
    outerBody.addRectangle(outerBodyRect);
    innerBody.addRectangle(innerBodyRect);
    dial.addRectangle(dialRect);
    
    g.setColour(juce::Colours::black);
    g.fillPath(outerBody);

    g.setColour(juce::Colours::antiquewhite);
    g.fillPath(innerBody);

    g.setColour(juce::Colours::black);
    g.fillPath(dial);
}
};
