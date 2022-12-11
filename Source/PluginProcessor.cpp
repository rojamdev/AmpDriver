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
    level = 1.0f;
    drive = 1.0f;

    // Filter setup    
    numChannels = getTotalNumInputChannels();

    lowPassFilters.resize(numChannels);
    highPassFilters.resize(numChannels);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        lowPassFilters[channel] = std::make_unique<FirstOrderIIR>(LPF_DEFAULT, false);
        highPassFilters[channel] = std::make_unique<FirstOrderIIR>(HPF_DEFAULT, true);
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

    params.add(std::make_unique<juce::AudioParameterFloat>(LPF_ID,
                                                           LPF_NAME,
                                                           juce::NormalisableRange<float>(LPF_MIN,
                                                                                          LPF_MAX,
                                                                                          LPF_INTERVAL),
                                                           LPF_DEFAULT));

    params.add(std::make_unique<juce::AudioParameterFloat>(HPF_ID,
                                                           HPF_NAME,
                                                           juce::NormalisableRange<float>(HPF_MIN,
                                                                                          HPF_MAX,
                                                                                          HPF_INTERVAL),
                                                           HPF_DEFAULT));

    return params;
}

void AmpDriverAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == LEVEL_ID)
        level = dBtoRatio(newValue);

    else if (parameterID == DRIVE_ID)
        drive = dBtoRatio(newValue);

    else if (parameterID == LPF_ID)
        for (int channel = 0; channel < numChannels; ++channel)
            lowPassFilters[channel]->setAlpha(LPF_DEFAULT);

    else if (parameterID == HPF_ID)
        for (int channel = 0; channel < numChannels; ++channel)
            highPassFilters[channel]->setAlpha(HPF_DEFAULT);
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
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
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
    for (int channel = 0; channel < numChannels; ++channel)
    {
        lowPassFilters[channel]->reset();
        highPassFilters[channel]->reset();
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
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // level = dBtoRatio(*apvts.getRawParameterValue("level"));

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        auto bufferLength = buffer.getNumSamples();

        for (int sample = 0; sample < bufferLength; ++sample)
        {
            channelData[sample] = lowPassFilters[channel]->processSample(channelData[sample]);
            channelData[sample] = highPassFilters[channel]->processSample(channelData[sample]);

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
