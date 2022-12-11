#pragma once

#include <JuceHeader.h>
#include "FirstOrderIIR.h"
#include "Constants.h"

typedef juce::AudioProcessorValueTreeState::ParameterLayout ParameterLayout;

//==============================================================================
class AmpDriverAudioProcessor  : public juce::AudioProcessor, 
                                 public juce::AudioProcessorValueTreeState::Listener,
                                 public juce::ChangeBroadcaster
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    AmpDriverAudioProcessor();
    ~AmpDriverAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;
    
private:
    //==============================================================================
    ParameterLayout createParameterLayout();
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    inline float dBtoRatio(float dB) 
    { 
        return pow(10, dB / 20); 
    }

    //==============================================================================
    float level, drive;
    std::vector<std::unique_ptr<FirstOrderIIR>> lowPassFilters, highPassFilters;

    int numChannels;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpDriverAudioProcessor)
};
