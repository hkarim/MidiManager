//
//  MessageBus.h
//  MidiManager
//
//  Created by Hossam Karim on 9/3/14.
//
//

#include <vector>
#include <string>

#ifndef __MidiManager__MessageBus__
#define __MidiManager__MessageBus__


enum class UIEvent : unsigned int {
    EditorLoaded,
    EditorResized,
    EditorRepaint,
    EditorRestoreState,
    FileChosen,
    ScriptErrors
};

struct Event {
    UIEvent uiEvent;
    
    struct {
        std::string filename;
    } FileData ;
    
    struct {
        int w;
        int h;
    } ViewCoordinates ;
    
    struct {
        std::string compilationErrors;
    } ScriptData ;
};

class EventListener {
public:
    EventListener() {}
    virtual ~EventListener() {} ;
    virtual void onEvent(const Event& event) = 0;
};

class MessageBus {
private:
    std::vector<EventListener*> listeners;
    MessageBus(MessageBus const&) =delete;     // Don't Implement
    void operator=(MessageBus const&) =delete; // Don't implement
public:
    MessageBus() : listeners {} {}
    void addListener(EventListener* listener) {
        //printf("adding listener: %p\n", listener);
        listeners.push_back(listener);
    }
    
    void removeListener(EventListener* listener) {
        //printf("removing listener: %p\n", listener);
        auto itr = std::find(listeners.begin(), listeners.end(), listener);
        listeners.erase(itr);
    }
    
    void publish(const Event& event) {
        //printf("publishing event, listeners size = %lu\n", listeners.size());
        for (auto l : listeners) {
            l->onEvent(event);
        }
    }
    
};



#endif /* defined(__MidiManager__MessageBus__) */
