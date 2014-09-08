//
//  PureTest.h
//  MidiManager
//
//  Created by Hossam Karim on 8/30/14.
//
//

#ifndef __MidiManager__PureTest__
#define __MidiManager__PureTest__

#include <stdio.h>
#include <mutex>
#include <string>
#include <vector>
#include <pure/runtime.h>
#include "../JuceLibraryCode/JuceHeader.h"
#include "PureEditor.h"
#include "MessageBus.h"


class PureLink : EventListener {
private:
    std::mutex mutex;
    MessageBus* bus;
    pure_interp* interp = nullptr;
    pure_expr* block = nullptr;
    pure_expr* processMidiBuffer = nullptr;
    pure_expr* createEditor = nullptr;
    
    
    PureEditor* editor = nullptr;
    pure_expr* editorExpr = nullptr;

    std::string filename;
    std::string code;
    std::string errors;
    bool debug = false;
    bool logging = false;
    bool silenceOnErrors = false;
    
    
public:
    PureLink(const std::string& filename, MessageBus* bus);
    ~PureLink();
    void onEvent(const Event& event);
    static void callPureFinalize();
    PureEditor* getPureEditor();
    const std::string getFilename();
    MidiBuffer processBlock(MidiBuffer& input);
    bool hasErrors();
    const std::string getErrors();
    bool isLogging() { return logging; }
    void setLogging(bool l) { logging = l; }
    bool isSilenceOnErrors() { return silenceOnErrors; }
    void setSilenceOnErrors(bool s) { silenceOnErrors = s; }
    const pure_interp* currentInterpreter() { return pure_current_interp(); }

private:
    void init();
    void addGuiHook();
    pure_expr* createNoteOnMessage(int channel, int note, int velocity, int position);
    pure_expr* createNoteOffMessage(int channel, int note, int position);
    bool createMessageFrom(pure_expr* expr, MidiMessage& message, int& position);
};



#endif /* defined(__MidiManager__PureTest__) */
