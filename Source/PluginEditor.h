#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

//==============================================================================
class AmpDriverAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AmpDriverAudioProcessorEditor (AmpDriverAudioProcessor&);
    ~AmpDriverAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    void createControl(juce::String parameterID, 
                       juce::String parameterName,
                       juce::Slider& slider,
                       juce::Label& label,
                       std::unique_ptr<SliderAttachment>& attachment,
                       int posX, int posY, 
                       int height, int width);

    //==============================================================================
    AmpDriverAudioProcessor& audioProcessor;

    CustomLookAndFeel lookAndFeel;
    juce::Font defaultFont;

    juce::Label levelLabel, driveLabel, lpfLabel, hpfLabel;
    juce::Slider levelSlider, driveSlider, lpfSlider, hpfSlider;

    std::unique_ptr<SliderAttachment> 
        levelAttachment, driveAttachment, lpfAttachment, hpfAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpDriverAudioProcessorEditor)
};
