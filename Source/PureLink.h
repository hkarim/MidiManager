//
//  PureTest.h
//  MidiManager
//
//  Created by Hossam Karim on 8/30/14.
//
//

#ifndef __MidiManager__PureTest__
#define __MidiManager__PureTest__

#include "../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>
#include <mutex>
#include <string>
#include <map>
#include <vector>
#include <pure/runtime.h>
#include "MessageBus.h"



struct WidgetAction {
    UIWidget* widget;
    pure_expr* action;
};


class PureLink : EventListener {
private:
    std::mutex mutex;
    MessageBus* bus;
    pure_interp* interp = nullptr;
    pure_expr* block = nullptr;
    pure_expr* processMidiBuffer = nullptr;
    pure_expr* createUI = nullptr;
    std::string filename;
    std::string code;
    std::string errors;
    bool debug = true;
    bool logging = false;
    bool silenceOnErrors = false;
    
    std::map<int, WidgetAction*> widgetMap;
    std::vector<UIWidget*> widgetVector;
    
        
    
public:
    PureLink(const std::string& filename, MessageBus* bus);
    ~PureLink();
    void onEvent(const Event& event);
    static void callPureFinalize();
    const std::string getFilename();
    const std::vector<UIWidget*>& getWidgets() const { return widgetVector; }
    MidiBuffer processBlock(MidiBuffer& input);
    bool hasErrors();
    const std::string getErrors();
    bool isLogging() { return logging; }
    void setLogging(bool l) { logging = l; }
    bool isSilenceOnErrors() { return silenceOnErrors; }
    void setSilenceOnErrors(bool s) { silenceOnErrors = s; }
    const pure_interp* currentInterpreter() { return pure_current_interp(); }
    void log(const std::string& ingoing, const std::string& outgoing);
private:
    void init();
    pure_expr* createNoteOnMessage(int channel, int note, int velocity, int position);
    pure_expr* createNoteOffMessage(int channel, int note, int position);
    bool createMessageFrom(pure_expr* expr, MidiMessage& message, int& position);
    
    void initUI();
    pure_expr* widgetExprProperty(const pure_expr* widget, const std::string& name);
    int widgetIntProperty(const pure_expr* widget, const std::string& name);
    std::string widgetStringProperty(const pure_expr* widget, const std::string& name);
    void onEditorWidgetChange(const Event& event);
    
    void report(pure_expr* exception);
    void report(std::string& message);
    
};



#endif /* defined(__MidiManager__PureTest__) */
