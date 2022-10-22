/*
  ==============================================================================

    This file was auto-generated and contains the startup code for a PIP.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "HostPluginDemo.h"

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HostAudioProcessor();
}
