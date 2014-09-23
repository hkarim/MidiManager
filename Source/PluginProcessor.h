/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <mutex>
#include "PureLink.h"
#include "JuceCocoa.h"
#include "MessageBus.h"
#include <memory>


//==============================================================================
/**
*/
class MidiManagerAudioProcessor  : public AudioProcessor, public EventListener
{
private:
    std::shared_ptr<MessageBus> bus;
    std::shared_ptr<JuceCocoa> juceCocoa;
    std::unique_ptr<PureLink> pureLink;
    std::mutex pureLinkMutex;
public:
    //==============================================================================
    MidiManagerAudioProcessor();
    ~MidiManagerAudioProcessor();
    std::shared_ptr<MessageBus> getMessageBus() { return bus; }
    std::shared_ptr<JuceCocoa> getJuceCocoa() { return juceCocoa; }
    //std::unique_ptr<PureLink> getPureLink() { return pureLink; }
    
    void onEvent(const Event& event);
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    //==============================================================================
    AudioProcessorEditor* createEditor();
    bool hasEditor() const;

    //==============================================================================
    const String getName() const;

    int getNumParameters();

    float getParameter (int index);
    void setParameter (int index, float newValue);

    const String getParameterName (int index);
    const String getParameterText (int index);

    const String getInputChannelName (int channelIndex) const;
    const String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;
    bool silenceInProducesSilenceOut() const;
    double getTailLengthSeconds() const;

    //==============================================================================
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);
    
    //==============================================================================

private:
    //==============================================================================
    void fileChosen(const Event& event);
    void restoreEditorState();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiManagerAudioProcessor)
};

#endif  // PLUGINPROCESSOR_H_INCLUDED
