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
    AmpDriverAudioProcessor& audioProcessor;

    CustomLookAndFeel lookAndFeel;
    juce::Font defaultFont;

    juce::Label levelLabel;
    juce::Slider levelSlider;
    std::unique_ptr<SliderAttachment> levelAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpDriverAudioProcessorEditor)
};
