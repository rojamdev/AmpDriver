#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AmpDriverAudioProcessorEditor::AmpDriverAudioProcessorEditor (AmpDriverAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (290, 390);
    setLookAndFeel(&lookAndFeel);
    defaultFont.setBold(true);

    createControl(LEVEL_ID, LEVEL_NAME, levelSlider, levelLabel, levelAttachment, 
                  BORDER_POS_X + 30, BORDER_POS_Y + 10, CONTROL_WIDTH, CONTROL_HEIGHT, false);

    createControl(DRIVE_ID, DRIVE_NAME, driveSlider, driveLabel, driveAttachment, 
                  160, BORDER_POS_Y + 10,
                  CONTROL_WIDTH, CONTROL_HEIGHT, false);

    createControl(LPF_ID, LPF_NAME, lpfSlider, lpfLabel, lpfAttachment, 50, 130, 80, 80, false);

    createControl(HPF_ID, HPF_NAME, hpfSlider, hpfLabel, hpfAttachment, 160, 130, 80, 80, false);
}

AmpDriverAudioProcessorEditor::~AmpDriverAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void AmpDriverAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::darkgrey);

    g.drawRoundedRectangle(BORDER_POS_X, 
                           BORDER_POS_Y, 
                           BORDER_WIDTH, 
                           BORDER_HEIGHT, 
                           BORDER_CORNER_SIZE,
                           BORDER_THICKNESS);

    g.setColour(juce::Colours::white);
    g.drawRoundedRectangle(60, 260, 170, 80, BORDER_CORNER_SIZE, BORDER_THICKNESS);
}

void AmpDriverAudioProcessorEditor::resized() {}

void AmpDriverAudioProcessorEditor::createControl(juce::String parameterID, 
                                                  juce::String parameterName,
                                                  juce::Slider& slider,
                                                  juce::Label& label,
                                                  std::unique_ptr<SliderAttachment>& attachment,
                                                  int posX, int posY, 
                                                  int height, int width,
                                                  bool showTextBox)
{   
    addAndMakeVisible(slider);
    attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, parameterID, slider);
    slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    slider.setBounds(posX, posY, height, width);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    juce::Slider::TextEntryBoxPosition boxPos;
    if (showTextBox) boxPos = juce::Slider::TextBoxBelow;
    else boxPos = juce::Slider::NoTextBox;
    slider.setTextBoxStyle(boxPos, true, 100, height / 5);

    addAndMakeVisible(label);
    label.setText(parameterName, juce::dontSendNotification);
    label.setBounds(posX, posY + height, width, height / 5);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(defaultFont);
}
