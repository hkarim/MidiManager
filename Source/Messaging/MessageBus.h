//
//  MessageBus.h
//  MidiManager
//
//  Created by Hossam Karim on 9/3/14.
//
//

#include <vector>
#include <list>
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
    virtual bool isValuePropertyInt() = 0;
    virtual bool isValuePropertyString() = 0;
    virtual int currentIntValue() { return 0; }
    virtual void setCurrentIntValue(int value) {}
    virtual std::string currentStringValue() { return ""; }
    virtual void setCurrentStringValue(const std::string& value) {}
};

struct UIWidgetSlider : UIWidget {
    UIWidgetSlider() : UIWidget {} { widgetType = UIWidgetType::Slider; }
    int minimum;
    int maximum;
    int value;
    virtual bool isValuePropertyInt() { return true; }
    virtual bool isValuePropertyString() { return false; }
    virtual int currentIntValue() { return value; }
    virtual void setCurrentIntValue(int value) { this->value = value; }
};
    
struct UIWidgetSegmented : UIWidget {
    UIWidgetSegmented() : UIWidget {} { widgetType = UIWidgetType::Segmented; }
    int value;
    std::vector<std::string> labels;
    virtual bool isValuePropertyInt() { return true; }
    virtual bool isValuePropertyString() { return false; }
    virtual int currentIntValue() { return value; }
    virtual void setCurrentIntValue(int value) { this->value = value; }
};

struct UIWidgetPopUp : UIWidget {
    UIWidgetPopUp() : UIWidget {} { widgetType = UIWidgetType::PopUp; }
    int value;
    std::vector<std::string> labels;
    virtual bool isValuePropertyInt() { return true; }
    virtual bool isValuePropertyString() { return false; }
    virtual int currentIntValue() { return value; }
    virtual void setCurrentIntValue(int value) { this->value = value; }
};
    
struct UIWidgetCheckBox : UIWidget {
    UIWidgetCheckBox() : UIWidget {} { widgetType = UIWidgetType::CheckBox; }
    int value;
    virtual bool isValuePropertyInt() { return true; }
    virtual bool isValuePropertyString() { return false; }
    virtual int currentIntValue() { return value; }
    virtual void setCurrentIntValue(int value) { this->value = value; }
};
    
struct UIWidgetLabel : UIWidget {
    UIWidgetLabel() : UIWidget {} { widgetType = UIWidgetType::Label; }
    std::string value;
    virtual bool isValuePropertyInt() { return false; }
    virtual bool isValuePropertyString() { return true; }
    virtual std::string currentStringValue() { return value; }
    virtual void setCurrentStringValue(const std::string& value) { this->value = value; }
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
    std::list<EventListener*> listeners;
    tp::ThreadPool pool;
    MessageBus(MessageBus const&) =delete;     // Don't Implement
    void operator=(MessageBus const&) =delete; // Don't implement
public:
    MessageBus() : listeners { }, pool { 8 } { }
    ~MessageBus() {
        printf("MessageBus %p freed\n", this);
    }
    void addListener(EventListener* listener) {
        //printf("<MessageBus-%p> adding listener: %p\n", this, listener);
        listeners.push_back(listener);
        //printf("<MessageBus-%p> current size: %lu\n", this, listeners.size());
    }
    
    void removeListener(EventListener* listener) {
        //printf("<MessageBus-%p> removing listener: %p\n", this, listener);
        listeners.erase(std::remove(listeners.begin(), listeners.end(), listener), listeners.end());
        //printf("<MessageBus-%p> current size: %lu\n", this, listeners.size());
    }
    
    void publish(const Event& event) {
        //printf("<MessageBus-%p> publishing event, listeners size = %lu\n", this, listeners.size());
        
        for (auto& l : listeners) {
            printf("<MessageBus-%p> invoking listener: %p\n", this, l);
            l->onEvent(event);
        }
        
    }
    
    void publishAsync(const Event& event) {
        //printf("<MessageBus-%p> async publishing event, listeners size = %lu\n", this, listeners.size());
        for (auto& l : listeners) {
            //printf("<MessageBus-%p> async invoking listener: %p\n", this, l);
            pool.enqueue([&](Event e) { l->onEvent(e); }, event);
        }
    }
    
};
    



#endif /* defined(__MidiManager__MessageBus__) */
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
