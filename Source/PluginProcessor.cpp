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
    sampleRate = 0.0;
    level = dBtoRatio(LEVEL_DEFAULT);
    drive = dBtoRatio(DRIVE_DEFAULT);
    bandpassQ = calcBandpassQ(DRIVE_DEFAULT);

    numChannels = getNumInputChannels();

    for (int channel = 0; channel < numChannels; channel++)
    {
        lowPassFilters.push_back(std::make_unique<Filter>());
        highPassFilters.push_back(std::make_unique<Filter>());
        bandpassFilters.push_back(std::make_unique<Filter>());
    }

    // Parameter listeners
    apvts.addParameterListener(LEVEL_ID, this);
    apvts.addParameterListener(DRIVE_ID, this);
    apvts.addParameterListener(LPF_ID, this);
    apvts.addParameterListener(HPF_ID, this);

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

    juce::NormalisableRange<float> hpfRange(HPF_MIN, HPF_MAX, HPF_INTERVAL);
    hpfRange.setSkewForCentre(HPF_DEFAULT);
    params.add(std::make_unique<juce::AudioParameterFloat>(HPF_ID,
                                                           HPF_NAME,
                                                           hpfRange,
                                                           HPF_DEFAULT));

    return params;
}

void AmpDriverAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == LEVEL_ID)
        level = dBtoRatio(newValue);

    else if (parameterID == DRIVE_ID)
    {
        drive = dBtoRatio(newValue);
        
        // Calculates higher values for the Q factor of pre-saturation bandpass filtering
        // for higher amounts of saturation and then calculates coefficients for bandpass filter
        for (int channel = 0; channel < numChannels; channel++)
            bandpassFilters[channel]->coefficients = Coefficients::makeBandPass(sampleRate,
                                                                                BANDPASS_FREQ,
                                                                                calcBandpassQ(newValue));
    }

    else if (parameterID == LPF_ID)
        for (int channel = 0; channel < numChannels; channel++)
            lowPassFilters[channel]->coefficients = Coefficients::makeLowPass(sampleRate, newValue);

    else if (parameterID == HPF_ID)
        for (int channel = 0; channel < numChannels; channel++)
            highPassFilters[channel]->coefficients = Coefficients::makeHighPass(sampleRate, newValue);
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

double AmpDriverAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int AmpDriverAudioProcessor::getNumPrograms() { return 1; }

int AmpDriverAudioProcessor::getCurrentProgram() { return 0; }

void AmpDriverAudioProcessor::setCurrentProgram (int index) {}

const juce::String AmpDriverAudioProcessor::getProgramName (int index) { return {}; }

void AmpDriverAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

//==============================================================================
void AmpDriverAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;

    for (int channel = 0; channel < numChannels; channel++)
    {
        lowPassFilters[channel]->coefficients = Coefficients::makeLowPass(sampleRate,
                                                                          *apvts.getRawParameterValue(LPF_ID));
        highPassFilters[channel]->coefficients = Coefficients::makeHighPass(sampleRate,
                                                                            *apvts.getRawParameterValue(HPF_ID));
        highPassFilters[channel]->coefficients = Coefficients::makeBandPass(sampleRate,
                                                                            BANDPASS_FREQ,
                                                                            calcBandpassQ(
                                                                                *apvts.getRawParameterValue(DRIVE_ID)
                                                                            ));
        lowPassFilters[channel]->reset();
        highPassFilters[channel]->reset();
        bandpassFilters[channel]->reset();
    }
}

void AmpDriverAudioProcessor::releaseResources()
{
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
            auto x = channelData[sample];

            x = highPassFilters[channel]->processSample(x);
            x = saturateSample(channel, x, drive);
            x = lowPassFilters[channel]->processSample(x);
            x *= level;

            channelData[sample] = x;
        }
    }
}

float AmpDriverAudioProcessor::saturateSample(int channel, float sample, float gain)
{
    auto x = sample;
    x = bandpassFilters[channel]->processSample(x);
    x = atan(gain * x) / sqrt(gain);
    return x;
}

//==============================================================================
bool AmpDriverAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AmpDriverAudioProcessor::createEditor()
{
    return new AmpDriverAudioProcessorEditor (*this);
}

//==============================================================================
void AmpDriverAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
}

void AmpDriverAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AmpDriverAudioProcessor();
}
