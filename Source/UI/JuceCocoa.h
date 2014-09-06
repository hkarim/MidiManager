//
//  JuceCocoa.h
//  MidiManager
//
//  Created by Hossam Karim on 9/2/14.
//
//

#ifndef MidiManager_JuceCocoa_h
#define MidiManager_JuceCocoa_h
#include "../JuceLibraryCode/JuceHeader.h"
#include "MessageBus.h"

class JuceCocoa : public EventListener {
private:
    void* controller = nullptr;
    void* view = nullptr;
    MessageBus* bus;
public:
    JuceCocoa(MessageBus* bus);
    ~JuceCocoa();
    void* createNSView();
    void onEvent(const Event& event);
};




#endif
