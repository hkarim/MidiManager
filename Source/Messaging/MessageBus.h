//
//  MessageBus.h
//  MidiManager
//
//  Created by Hossam Karim on 9/3/14.
//
//

#include <vector>
#include <string>
#include "ThreadPool.h"

#ifndef __MidiManager__MessageBus__
#define __MidiManager__MessageBus__


enum class UIEvent : unsigned int {
    EditorLoaded,
    EditorResized,
    EditorRepaint,
    EditorRestoreState,
    EditorConfigured,
    FileChosen,
    ScriptErrors,
    Logging
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
    
    struct {
        bool debug;
        bool silenceOnErrors;
    } Configuration;
    
    struct {
        std::string packet;
    } MidiStream;
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
    tp::ThreadPool pool;
    MessageBus(MessageBus const&) =delete;     // Don't Implement
    void operator=(MessageBus const&) =delete; // Don't implement
public:
    MessageBus() : listeners { }, pool { 8 } { }
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
    
    void publishAsync(const Event& event) {
        for (auto l : listeners) {
            pool.enqueue([=](Event e) { l->onEvent(e); }, event);
        }
    }
    
};



#endif /* defined(__MidiManager__MessageBus__) */
