#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AmpDriverAudioProcessor::AmpDriverAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    apvts(*this, nullptr, "PARAMS", createParameterLayout())
#endif
{
    numChannels = getNumInputChannels();
    sampleRate = getSampleRate();
    
    lowPassFilters.resize(numChannels);

    for (int channel = 0; channel < numChannels; channel++)
    {
        lowPassFilters[channel] = std::make_unique<Filter>();
    }

    level = 1.0f;
    drive = 1.0f;

    // Parameter listeners
    apvts.addParameterListener(LEVEL_ID, this);
    apvts.addParameterListener(DRIVE_ID, this);
    apvts.addParameterListener(LPF_ID, this);

    apvts.state = juce::ValueTree(JucePlugin_Name);
}

AmpDriverAudioProcessor::~AmpDriverAudioProcessor() {}

ParameterLayout AmpDriverAudioProcessor::createParameterLayout()
{
    ParameterLayout params;

    params.add(std::make_unique<juce::AudioParameterFloat>(LEVEL_ID,
                                                           LEVEL_NAME,
                                                           juce::NormalisableRange<float>(LEVEL_MIN,
                                                                                          LEVEL_MAX,
                                                                                          LEVEL_INTERVAL),
                                                           LEVEL_DEFAULT));

    params.add(std::make_unique<juce::AudioParameterFloat>(DRIVE_ID,
                                                           DRIVE_NAME,
                                                           juce::NormalisableRange<float>(DRIVE_MIN,
                                                                                          DRIVE_MAX,
                                                                                          DRIVE_INTERVAL),
                                                           DRIVE_DEFAULT));

    juce::NormalisableRange<float> lpfRange(LPF_MIN, LPF_MAX, LPF_INTERVAL);
    lpfRange.setSkewForCentre(LPF_DEFAULT);
    params.add(std::make_unique<juce::AudioParameterFloat>(LPF_ID,
                                                           LPF_NAME,
                                                           lpfRange,
                                                           LPF_DEFAULT));

    return params;
}

void AmpDriverAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == LEVEL_ID)
        level = dBtoRatio(newValue);

    else if (parameterID == DRIVE_ID)
        drive = dBtoRatio(newValue);

    else if (parameterID == LPF_ID)
    {
        for (int channel = 0; channel < numChannels; channel++)
            lowPassFilters[channel]->coefficients = Coefficients::makeLowPass(sampleRate, newValue);
    }
}


//==============================================================================
const juce::String AmpDriverAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AmpDriverAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AmpDriverAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AmpDriverAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AmpDriverAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AmpDriverAudioProcessor::getNumPrograms()
{
    return 1;
}

int AmpDriverAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AmpDriverAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AmpDriverAudioProcessor::getProgramName (int index)
{
    return {};
}

void AmpDriverAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AmpDriverAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;

    for (int channel = 0; channel < numChannels; channel++)
    {
        lowPassFilters[channel]->coefficients = Coefficients::makeLowPass(sampleRate,
                                                                          *apvts.getRawParameterValue(LPF_ID));
    }
}

void AmpDriverAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AmpDriverAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
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

void AmpDriverAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // Main processing loop
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        auto bufferLength = buffer.getNumSamples();

        for (int sample = 0; sample < bufferLength; ++sample)
        {
            channelData[sample] = lowPassFilters[channel]->processSample(channelData[sample]);
            
            channelData[sample] = atan(drive * channelData[sample]) / sqrt(drive);
            channelData[sample] *= level;
        }
    }
}

//==============================================================================
bool AmpDriverAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AmpDriverAudioProcessor::createEditor()
{
    return new AmpDriverAudioProcessorEditor (*this);
}

//==============================================================================
void AmpDriverAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AmpDriverAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AmpDriverAudioProcessor();
}
