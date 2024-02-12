/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicDelayAudioProcessorEditor::BasicDelayAudioProcessorEditor (BasicDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    timeSlider.setRange(0.0f, 1.7f);
    timeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    timeLabel.setText("Echo Time", juce::dontSendNotification);
    timeSlider.setValue(1.0f);

    //Add listener to timeSlider
    timeSlider.addListener(this);

    echoVolSlider.setRange(0.0f, .7f);
    echoVolSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    echoVolLabel.setText("Echo Volume", juce::dontSendNotification);
    echoVolSlider.setValue(0.1f);

    echoVolSlider.addListener(this);

    regenSlider.setRange(0.0f, .7f);
    regenSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    regenLabel.setText("Echo Regen", juce::dontSendNotification);
    regenSlider.setValue(0.1f);

    regenSlider.addListener(this);

    addAndMakeVisible(timeSlider);
    addAndMakeVisible(timeLabel);
    addAndMakeVisible(echoVolSlider);
    addAndMakeVisible(echoVolLabel);
    addAndMakeVisible(regenSlider);
    addAndMakeVisible(regenLabel);
    
    setSize (400, 300);
}

BasicDelayAudioProcessorEditor::~BasicDelayAudioProcessorEditor()
{
}

//==============================================================================
void BasicDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
}

void BasicDelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    timeLabel.setBounds(10, 10, 90, 20);
    timeSlider.setBounds(100, 10, getWidth() - 110, 20);
    echoVolLabel.setBounds(10, 40, 90, 20);
    echoVolSlider.setBounds(100, 40, getWidth() - 110, 20);
    regenLabel.setBounds(10, 70, 90, 20);
    regenSlider.setBounds(100, 70, getWidth() - 110, 20);
}

void BasicDelayAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &timeSlider)
        processor.getParameters()[0]->setValue(slider->getValue());
    else if (slider == &echoVolSlider)
        processor.getParameters()[1]->setValue(slider->getValue());
    else if (slider == &regenSlider)
        processor.getParameters()[2]->setValue(slider->getValue());
}
