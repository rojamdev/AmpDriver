#pragma once

#include <JuceHeader.h>

class ParameterValues
{
public:
    ParameterValues(juce::String parameterID,
                    juce::String parameterName,
                    float minValue,
                    float maxValue,
                    float defaultValue,
                    float intervalValue)
    {
        ID = parameterID;
        name = parameterName;
        defaultVal = defaultValue;
        
        juce::NormalisableRange<float> range(minValue, maxValue, intervalValue);
        range.setSkewForCentre(defaultVal);
    }

    juce::String getID() { return ID; }
    juce::String getName() { return name; }
    juce::NormalisableRange<float> getRange()  { return range; }
    float getDefault() { return defaultVal; }

private:
    juce::String ID, name;
    juce::NormalisableRange<float> range;
    float defaultVal;
};