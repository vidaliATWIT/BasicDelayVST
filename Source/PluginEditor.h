/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class BasicDelayAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Slider::Listener
{
public:
    BasicDelayAudioProcessorEditor (BasicDelayAudioProcessor&);
    ~BasicDelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged(juce::Slider* slider) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BasicDelayAudioProcessor& audioProcessor;
    juce::Label timeLabel;
    juce::Slider timeSlider;
    juce::Label echoVolLabel;
    juce::Slider echoVolSlider;
    juce::Label regenLabel;
    juce::Slider regenSlider;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BasicDelayAudioProcessorEditor)
};
