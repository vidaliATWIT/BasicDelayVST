/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicDelayAudioProcessor::BasicDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    time = new Echo_Parameter();
    time->defaultValue = 1.0f;
    time->currentValue = 1.0f;
    time->name = "Time";
    addParameter(time);

    volume = new Echo_Parameter();
    volume->defaultValue = 0.1f;
    volume->currentValue = 0.1f;
    volume->name = "Echo Volume";
    addParameter(volume);
    
    regen = new Echo_Parameter();
    regen->defaultValue = 0.1f;
    regen->currentValue = 0.1f;;
    regen->name = "Regen";
    addParameter(regen);

}

BasicDelayAudioProcessor::~BasicDelayAudioProcessor()
{
}

//==============================================================================
const juce::String BasicDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BasicDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BasicDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BasicDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BasicDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BasicDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BasicDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BasicDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void BasicDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BasicDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{ 
    auto delayBufferSize = 2.0f * sampleRate; //Our buffer is the size of 2 seconds worth of audio, for a 2 second delay
    mDelayBuffer.setSize(getTotalNumInputChannels(), (int) delayBufferSize);
    mDelayBuffer.clear();
}

void BasicDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void BasicDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const int bufferLength = buffer.getNumSamples();
    const int delayBufferLength = mDelayBuffer.getNumSamples();

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* bufferData = buffer.getWritePointer(channel);

        fillBuffer(channel, bufferLength, delayBufferLength, bufferData);
        
        readFromBuffer(channel, bufferLength, delayBufferLength, buffer);
        
        fillBuffer(channel, bufferLength, delayBufferLength, bufferData);

    }

    //DBG("Delay Bufer Size:" << delayBufferLength);
    //DBG("Buffer Size:" << bufferLength);
    //DBG("Write PositionL: " << mWritePosition);

    mWritePosition += bufferLength;
    mWritePosition %= delayBufferLength;
}

void BasicDelayAudioProcessor::fillBuffer(int channel, int bufferLength, int delayBufferLength, float* bufferData)
{
    //copy the data from main buffer to delay buffer
    if (delayBufferLength > bufferLength + mWritePosition)
    {
        mDelayBuffer.copyFrom(channel, mWritePosition, bufferData, bufferLength);
    }
    else
    {
        auto numSamplesToEnd = delayBufferLength - mWritePosition;
        //copy just alittle bit of whats left to fillout the end
        mDelayBuffer.copyFrom(channel, mWritePosition, bufferData, numSamplesToEnd);

        auto numSamplesAtStart = bufferLength - numSamplesToEnd;
        //circle back and add whats left to the beginning again
        mDelayBuffer.copyFrom(channel, 0, bufferData + numSamplesToEnd, numSamplesAtStart);
    }
}

//Add audio back into main buffer
void BasicDelayAudioProcessor::readFromBuffer(int channel,int bufferLength, int delayBufferLength, juce::AudioBuffer<float>& buffer)
{
    //original auto readPosition = mWritePosition - getSampleRate();
    auto readPosition = mWritePosition - (getSampleRate()*time->getValue());
    if (readPosition < 0)
        readPosition += delayBufferLength;

    auto g = volume->getValue();
    //
    if (readPosition + bufferLength < delayBufferLength)
    {
        buffer.addFromWithRamp(channel, 0, mDelayBuffer.getReadPointer(channel, readPosition), bufferLength, g, g);
    }
    else
    {
        auto numSamplesToEnd = delayBufferLength - readPosition;
        buffer.addFromWithRamp(channel, 0, mDelayBuffer.getReadPointer(channel, readPosition), numSamplesToEnd, g, g);

        auto numSamplesAtStart = bufferLength - numSamplesToEnd;
        buffer.addFromWithRamp(channel, numSamplesToEnd, mDelayBuffer.getReadPointer(channel, 0), numSamplesAtStart, g, g);
    }
}

//==============================================================================
bool BasicDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BasicDelayAudioProcessor::createEditor()
{
    return new BasicDelayAudioProcessorEditor (*this);
}

//==============================================================================
void BasicDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BasicDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicDelayAudioProcessor();
}
