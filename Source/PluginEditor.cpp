#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AmpDriverAudioProcessorEditor::AmpDriverAudioProcessorEditor (AmpDriverAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 500);
    setLookAndFeel(&lookAndFeel);
    defaultFont.setBold(true);

    createControl(LEVEL_ID, LEVEL_NAME, levelSlider, levelLabel, levelAttachment, 0, 0, 100, 100);
    createControl(DRIVE_ID, DRIVE_NAME, driveSlider, driveLabel, driveAttachment, 100, 0, 100, 100);
    createControl(LPF_ID, LPF_NAME, lpfSlider, lpfLabel, lpfAttachment, 200, 0, 100, 100);
    createControl(HPF_ID, HPF_NAME, hpfSlider, hpfLabel, hpfAttachment, 300, 0, 100, 100);
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

void AmpDriverAudioProcessorEditor::createControl(juce::String parameterID, 
                                                  juce::String parameterName,
                                                  juce::Slider& slider,
                                                  juce::Label& label,
                                                  std::unique_ptr<SliderAttachment>& attachment,
                                                  int posX, int posY, 
                                                  int height, int width)
{   
    addAndMakeVisible(slider);
    attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, parameterID, slider);
    slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, height / 5);
    slider.setBounds(posX, posY, height, width);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    addAndMakeVisible(label);
    label.setText(parameterName, juce::dontSendNotification);
    label.setBounds(posX, posY + height, width, height / 5);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(defaultFont);
}
