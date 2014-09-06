/*
  ==============================================================================

    This file was auto-generated by the Introjucer!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PureLink.h"
#include "JuceCocoa.h"




//==============================================================================
MidiManagerAudioProcessorEditor::MidiManagerAudioProcessorEditor (MidiManagerAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter), EventListener(), nsviewComponent()
{

    bus = ownerFilter->getMessageBus();
    bus->addListener(this);
    
    addAndMakeVisible(nsviewComponent);
    void* view = ownerFilter->getJuceCocoa()->createNSView();
    nsviewComponent.setView(view);
    setSize(currentWidth, currentHeight);
    
    
    
}

MidiManagerAudioProcessorEditor::~MidiManagerAudioProcessorEditor()
{
    //MidiManagerAudioProcessor* processor = dynamic_cast<MidiManagerAudioProcessor*>(getAudioProcessor());
    bus->removeListener(this);
}

void MidiManagerAudioProcessorEditor::resized() {
    //MidiManagerAudioProcessor* processor = dynamic_cast<MidiManagerAudioProcessor*>(getAudioProcessor());
    
    nsviewComponent.setBounds(0, 0, currentWidth, currentHeight);
    
    
}

void MidiManagerAudioProcessorEditor::onEvent(const Event& event) {
    switch (event.uiEvent) {
        case UIEvent::EditorResized:
            this->currentWidth = event.ViewCoordinates.w;
            this->currentHeight = event.ViewCoordinates.h;
            this->resized();
            this->repaint();
            break;
            
        default:
            break;
    }
    
}






//==============================================================================

void MidiManagerAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll(Colour::fromRGB(239, 239, 239));
}

