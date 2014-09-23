/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/
#include <thread>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <pure/runtime.h>



//==============================================================================
MidiManagerAudioProcessor::MidiManagerAudioProcessor(): EventListener() {
    printf("MidiManagerAudioProcessor: %p\n", this);

    bus = std::shared_ptr<MessageBus> { new MessageBus() };
    pureLink = std::unique_ptr<PureLink> { nullptr };
    juceCocoa = std::shared_ptr<JuceCocoa> { new JuceCocoa(bus) };
    bus->addListener(this);
    
}

MidiManagerAudioProcessor::~MidiManagerAudioProcessor()
{
    bus->removeListener(this);
    /*
    if (juceCocoa) {
        delete juceCocoa;
    }
    if (pureLink) {
        delete pureLink;
    }
    delete bus;
    */
    PureLink::callPureFinalize();
    printf("MidiManagerAudioProcessor %p freed\n", this);
}

void MidiManagerAudioProcessor::onEvent(const Event& event) {
    //printf("MidiManagerAudioProcessor::onEvent\n");
    switch (event.uiEvent) {
        case UIEvent::FileChosen:
            fileChosen(event);
            break;
        case UIEvent::EditorLoaded:
            restoreEditorState();
            break;
        default:
            break;
    }
}

void MidiManagerAudioProcessor::restoreEditorState() {
    if (!pureLink) {
        return;
    }
    
    Event e;
    e.uiEvent = UIEvent::EditorRestoreState;
    
    if (pureLink->hasErrors()) {
        e.FileData.filename = pureLink->getFilename();
        e.ScriptData.compilationErrors = pureLink->getErrors();
        bus->publish(e);
    } else {
        e.ScriptData.compilationErrors = "";
        e.FileData.filename = pureLink->getFilename();
        auto widgets = pureLink->getWidgets();
        if (!widgets.empty()) {
            e.UI.widgets = widgets;
        }
        bus->publish(e);
        pureLink->scriptEditorLoaded();
    }
    
    
}

void MidiManagerAudioProcessor::fileChosen(const Event& event) {
    auto filename = event.FileData.filename;
    
    if (filename.empty()) return;
    
    //printf("MidiManagerAudioProcessor: %p ::fileChoosen: %s\n", (void*) this, filename.c_str());
    
    {
        std::lock_guard<std::mutex> lock(pureLinkMutex);
        
        pureLink.reset(new PureLink(filename, bus));
    
        //pureLink = new PureLink(filename, bus);
        
        restoreEditorState();
    }
    
}


//==============================================================================
const String MidiManagerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int MidiManagerAudioProcessor::getNumParameters()
{
    return 1;
}

float MidiManagerAudioProcessor::getParameter (int index)
{
    return 0.0f;
}

void MidiManagerAudioProcessor::setParameter (int index, float newValue)
{
}

const String MidiManagerAudioProcessor::getParameterName (int index)
{
    return "Filename";
}

const String MidiManagerAudioProcessor::getParameterText (int index)
{
    return "Filename";
}

const String MidiManagerAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String MidiManagerAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool MidiManagerAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool MidiManagerAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool MidiManagerAudioProcessor::acceptsMidi() const
{
    return true;
}

bool MidiManagerAudioProcessor::producesMidi() const
{
    return true;
}

bool MidiManagerAudioProcessor::silenceInProducesSilenceOut() const
{
    return true;
}

double MidiManagerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MidiManagerAudioProcessor::getNumPrograms()
{
    return 1;
}

int MidiManagerAudioProcessor::getCurrentProgram()
{
    return 1;
}

void MidiManagerAudioProcessor::setCurrentProgram (int index)
{
}

const String MidiManagerAudioProcessor::getProgramName (int index)
{
    return "Default";
}

void MidiManagerAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void MidiManagerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    //filterGraph.getGraph().prepareToPlay(sampleRate, samplesPerBlock);

}

void MidiManagerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void MidiManagerAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    //MidiBuffer out;
    bool noErrors = pureLink && !pureLink->hasErrors();
    
    std::lock_guard<std::mutex> lock(pureLinkMutex);
    if (noErrors) {
        midiMessages = std::move(pureLink->processBlock(midiMessages));
    }
}

//==============================================================================
bool MidiManagerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* MidiManagerAudioProcessor::createEditor()
{
    return new MidiManagerAudioProcessorEditor (this);
}

//==============================================================================
void MidiManagerAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    //printf("MidiManagerAudioProcessor::getStateInformation\n");
    if (pureLink && !pureLink->getFilename().empty()) {
        //printf("MidiManagerAudioProcessor::getStateInformation: %s\n", pureLink->getFilename().c_str());
        ValueTree tree { String { "DefaultValueTree" } };
        MemoryOutputStream out { destData , false };
        tree.setProperty(Identifier { "filename" }, var { String(pureLink->getFilename()) }, nullptr);
        tree.setProperty(Identifier { "pureState" }, var { String(pureLink->getState()) }, nullptr);
        tree.writeToStream(out);
    }
    
}

void MidiManagerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    //printf("MidiManagerAudioProcessor::setStateInformation\n");
    ValueTree tree = ValueTree::readFromData(data, sizeInBytes);
    const var filename = tree.getProperty(Identifier { "filename" } );
    const var pureState = tree.getProperty(Identifier { "pureState" } );
    if (filename.isString()) {
        std::string name = filename.toString().toStdString();
        //printf("MidiManagerAudioProcessor::setStateInformation: %s\n", name.c_str());
        
        Event e;
        e.uiEvent = UIEvent::FileChosen;
        e.FileData.filename = name;
        //bus->publish(e);
        fileChosen(e);
    }
    
    if (pureState.isString()) {
        std::string state = pureState.toString().toStdString();
        if (pureLink) {
            pureLink->setState(state);
        }
    }
    
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    MidiManagerAudioProcessor* processor = new MidiManagerAudioProcessor();
    //printf("Plugin Instance: %p\n", (void*) processor);
    //printf("MessageBus Instance: %p\n", (void*) processor->getMessageBus());
    //printf("Plugin Pure Instance: %p\n", (void*) processor->getPureLink());
    //printf("Pure Interpreter Instance: %p\n", (void*) processor->getPureLink()->currentInterpreter());

    return processor;
}









