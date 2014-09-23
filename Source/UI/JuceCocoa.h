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
#include <memory>

class JuceCocoa : public EventListener {
private:
    void* controller = nullptr;
    void* view = nullptr;
    std::shared_ptr<MessageBus> bus;
public:
    JuceCocoa(std::shared_ptr<MessageBus> bus);
    ~JuceCocoa();
    void* createNSView();
    void onEvent(const Event& event);
    void releaseController();
    double currentHeight();
    double currentWidth();
};




#endif
