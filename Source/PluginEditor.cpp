#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AmpDriverAudioProcessorEditor::AmpDriverAudioProcessorEditor (AmpDriverAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
    setLookAndFeel(&lookAndFeel);
    defaultFont.setBold(true);
    
    levelAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "level", levelSlider);
    addAndMakeVisible(levelSlider);
    levelSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    levelSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 20);
    levelSlider.setBounds(0, 0, 100, 100);

    levelLabel.setText("Level", juce::dontSendNotification);
    addAndMakeVisible(levelLabel);
    levelLabel.setBounds(0, 100, 100, 30);
    levelLabel.setJustificationType(juce::Justification::centred);
}

AmpDriverAudioProcessorEditor::~AmpDriverAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void AmpDriverAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::black);
}

void AmpDriverAudioProcessorEditor::resized()
{

}
