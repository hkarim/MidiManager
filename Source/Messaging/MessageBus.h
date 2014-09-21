//
//  MessageBus.h
//  MidiManager
//
//  Created by Hossam Karim on 9/3/14.
//
//

#include <vector>
#include <string>
#include <mutex>
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
    Logging,
    EditorWidgetChanged,
    UISignal
    //CreateUI
};

enum class UIWidgetType : int {
    Unkown = -1, Slider = 100, Segmented = 101, PopUp = 102, CheckBox = 103, Label = 104
};

inline UIWidgetType uiWidgetType(const int tag) {
    switch (tag) {
        case 100:
            return UIWidgetType::Slider;
            break;
        case 101:
            return UIWidgetType::Segmented;
            break;
        case 102:
            return UIWidgetType::PopUp;
            break;
        case 103:
            return UIWidgetType::CheckBox;
            break;
        case 104:
            return UIWidgetType::Label;
            break;
            
        default:
            return UIWidgetType::Unkown;
            break;
    }
}

struct UIWidget {
    UIWidgetType widgetType;
    int code;
    std::string name;
    virtual ~UIWidget() {}
    virtual int currentIntValue() { return 0; }
    virtual std::string currentStringValue() { return ""; }
};

struct UIWidgetSlider : UIWidget {
    UIWidgetSlider() : UIWidget {} { widgetType = UIWidgetType::Slider; }
    int minimum;
    int maximum;
    int value;
    virtual int currentIntValue() { return value; }
};
    
struct UIWidgetSegmented : UIWidget {
    UIWidgetSegmented() : UIWidget {} { widgetType = UIWidgetType::Segmented; }
    int value;
    std::vector<std::string> labels;
    virtual int currentIntValue() { return value; }
};

struct UIWidgetPopUp : UIWidget {
    UIWidgetPopUp() : UIWidget {} { widgetType = UIWidgetType::PopUp; }
    int value;
    std::vector<std::string> labels;
    virtual int currentIntValue() { return value; }
};
    
struct UIWidgetCheckBox : UIWidget {
    UIWidgetCheckBox() : UIWidget {} { widgetType = UIWidgetType::CheckBox; }
    int value;
    virtual int currentIntValue() { return value; }
};
    
struct UIWidgetLabel : UIWidget {
    UIWidgetLabel() : UIWidget {} { widgetType = UIWidgetType::Label; }
    std::string value;
    virtual std::string currentStringValue() { return value; }
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
        std::string input;
        std::string output;
    } Logging;
    
    struct {
        std::vector<UIWidget*> widgets;
    } UI;
    
    struct {
        int code;
        int intValue;
        std::string stringValue;
    } Change;
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
        //printf("<MessageBus-%p> adding listener: %p\n", this, listener);
        listeners.push_back(listener);
        //printf("<MessageBus-%p> current size: %lu\n", this, listeners.size());
    }
    
    void removeListener(EventListener* listener) {
        //printf("<MessageBus-%p> removing listener: %p\n", this, listener);
        auto itr = std::find(listeners.begin(), listeners.end(), listener);
        listeners.erase(itr);
        //printf("<MessageBus-%p> current size: %lu\n", this, listeners.size());
    }
    
    void publish(const Event& event) {
        //printf("<MessageBus-%p> publishing event, listeners size = %lu\n", this, listeners.size());
        /*
        for (auto& l : listeners) {
            printf("<MessageBus-%p> invoking listener: %p\n", this, l);
            l->onEvent(event);
        }
         */
        
        for (int i = 0; i < listeners.size(); i++) {
            //printf("<MessageBus-%p> invoking listener: %p\n", this, listeners[i]);
            listeners[i]->onEvent(event);
        }
    }
    
    void publishAsync(const Event& event) {
        //printf("<MessageBus-%p> async publishing event, listeners size = %lu\n", this, listeners.size());
        for (auto& l : listeners) {
            //printf("<MessageBus-%p> async invoking listener: %p\n", this, l);
            pool.enqueue([=](Event e) { l->onEvent(e); }, event);
        }
    }
    
};



#endif /* defined(__MidiManager__MessageBus__) */
