#include "PluginEditor.h"

MyPluginAudioProcessorEditor::MyPluginAudioProcessorEditor(MyPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(400, 300);
}

void MyPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("My Plugin", getLocalBounds(), juce::Justification::centred, 1);
}

void MyPluginAudioProcessorEditor::resized()
{
    // Layout your components here
}
