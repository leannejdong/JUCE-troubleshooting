#include "PluginProcessor.h"
#include "PluginEditor.h"

MyPluginAudioProcessor::MyPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

void MyPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    processorChain.prepare(spec);
}

void MyPluginAudioProcessor::releaseResources()
{
    // Release any resources here
}

void MyPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Process audio
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    processorChain.process(context);
}

juce::AudioProcessorEditor* MyPluginAudioProcessor::createEditor()
{
    return new MyPluginAudioProcessorEditor(*this);
}

void MyPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Save parameters here
    juce::ignoreUnused(destData);
}

void MyPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore parameters here
    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MyPluginAudioProcessor();
}
