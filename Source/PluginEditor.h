#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class MyPluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit MyPluginAudioProcessorEditor(MyPluginAudioProcessor&);
    ~MyPluginAudioProcessorEditor() override = default;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MyPluginAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MyPluginAudioProcessorEditor)
};
