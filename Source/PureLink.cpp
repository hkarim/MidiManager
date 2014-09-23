//
//  PureLink.cpp
//  MidiManager
//
//  Created by Hossam Karim on 9/1/14.
//
//

#include "PureLink.h"
#include <fstream>
#include <sstream>
#include <map>
#include <sys/stat.h>



struct MainInterpreter {
    
    pure_interp* createInterpreter(bool debug) {
        const char* currentEx =
          File::getSpecialLocation(File::SpecialLocationType::currentExecutableFile).getFullPathName().toRawUTF8();
        std::string location =
          File::getSpecialLocation(File::SpecialLocationType::currentApplicationFile).getFullPathName().toStdString();
        std::string includeDir =
          std::string(location).append("/Contents/Resources/");
        std::string libDir =
          std::string(location).append("/Contents/MacOS/");
        
        std::vector<const char*> args;
        args.push_back(currentEx);
        args.push_back("--eager-jit");
        args.push_back("-I");
        args.push_back(includeDir.c_str());
        args.push_back("-L");
        args.push_back(libDir.c_str());
        if (debug) args.push_back("-g");
        args.push_back(nullptr);
        /*
        char* argv[] = {
            strdup(currentEx),
            strdup("--eager-jit"),
            strdup("-I"),
            strdup(includeDir.c_str()),
            strdup("-L"),
            strdup(libDir.c_str()),
            nullptr
        };
        */
        //printf("creating new interpreter instance\n");
        pure_interp* local = pure_lock_interp(defaultInstance);
        char** argv = (char**) &args[0];
        pure_interp* newInstance = pure_create_interp(args.size(), argv);
        pure_unlock_interp(local);
        return newInstance;
    }
    
    pure_interp* defaultInstance = pure_create_interp(0, 0);
    
} interpreter;




inline bool exists (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}


// native link support
extern "C"
void pure_link_debug(pure_expr* pureLinkPointer, const char* message) {
    //printf("pure_link_debug: %s\n", message);
    void* p;
    if (pure_is_pointer(pureLinkPointer, &p)) {
        //printf("passed\n");
        PureLink* pureLink = (PureLink*) p;
        if (pureLink)
            pureLink->log("[debug]", message);
    }
}

extern "C"
void pure_link_signal_int(pure_expr* pureLinkPointer, int widgetCode, int newValue) {
    //printf("pure_link_signal\n");
    void* p;
    if (pure_is_pointer(pureLinkPointer, &p)) {
        //printf("pure_link_signal pure_is_pointer passed\n");
        PureLink* pureLink = (PureLink*) p;
        if (pureLink)
            pureLink->signal(widgetCode, newValue);
    }
}

extern "C"
void pure_link_signal_str(pure_expr* pureLinkPointer, int widgetCode, const char* newValue) {
    //printf("pure_link_signal\n");
    void* p;
    if (pure_is_pointer(pureLinkPointer, &p)) {
        //printf("pure_link_signal pure_is_pointer passed\n");
        PureLink* pureLink = (PureLink*) p;
        if (pureLink)
            pureLink->signal(widgetCode, newValue);
    }
}

void message_bus_publish(pure_expr* messageBusPointer) {
    
}



const std::string processMidiBufferF = "process_midi_buffer";
const std::string isMidiBufferTF     = "is_midi_buffer";
const std::string createUIF          = "create_ui";
const std::string saveStateF         = "save_state";
const std::string restoreStateF      = "restore_state";
const std::string onEditorLoadedF    = "on_editor_loaded";
const std::string pureLinkVar        = "pure_link";
const std::string messageBusVar      = "message_bus";


const std::string typeTagProperty    = "ui::type_tag" ;
const std::string codeProperty       = "ui::code" ;
const std::string nameProperty       = "ui::name" ;
const std::string actionProperty     = "ui::action";
const std::string valueProperty      = "ui::value";
const std::string setValueF          = "ui::set_value";
const std::string allWidgetsF        = "ui::all_widgets";
const std::string isWidgetF          = "ui::is_widget";



PureLink::PureLink(const std::string& filename, std::shared_ptr<MessageBus> bus) :
EventListener { }, bus { bus }, filename { filename } {
    printf("PureLink: %p\n", this);
    if (debug)
        printf("PureLink: Loading pure file: %s\n", filename.c_str());

    std::lock_guard<std::mutex> lock(mutex);
        
    bus->addListener(this);
    
    if (exists(filename)) {
        std::ifstream stream(filename);
        std::stringstream buffer;
        buffer << stream.rdbuf();
        this->code = buffer.str();
        stream.close();
        init();
    } else {
        errors = "File doesn't exist";
    }
}

PureLink::~PureLink() {
    std::lock_guard<std::mutex> lock(mutex);
    
    bus->removeListener(this);
    
    
    if (debug) printf("PureLink: destroying pure link\n");
    
    if (debug) printf("deleting widgets vector\n");
    
    for (auto& p : widgetMap) {
        WidgetAction* wa = p.second;
        if (wa) delete wa;
    }
    
    widgetMap.clear();
    
    if (debug) printf("deleting blocks\n");
    /*
    if (processMidiBuffer) pure_free(processMidiBuffer);
    if (createUI) pure_free(createUI);
    if (pureLinkBinding) pure_free(pureLinkBinding);
    if (block) pure_free(block);
    pure_interp* defaultInstance = interpreter.defaultInstance;
    if (interp) {
        pure_interp* local = pure_lock_interp(defaultInstance);
        pure_delete_interp(interp);
        pure_unlock_interp(local);
    }
    */
    printf("PureLink %p freed\n", this);
}

void PureLink::init() {
    if (debug) printf("PureLink: init\n");
    if (code.empty()) {
        return;
    }

    pure_interp* local;
    interp.reset(interpreter.createInterpreter(debug));
    local = pure_lock_interp(interp.get());
    if (debug) printf("Pure Interpreter: %p\n", interp.get());
    pure_expr* evaluated = pure_eval(code.c_str());
    if (evaluated) {
        block.reset(pure_new(evaluated));
        
        
        int32_t processMidiBufferSymbol = pure_getsym(processMidiBufferF.c_str());
        if (processMidiBufferSymbol != 0 ) {
            processMidiBuffer.reset(pure_new(pure_symbol(processMidiBufferSymbol)));
            pure_interp_compile(interp.get(), processMidiBufferSymbol);
        }
        
        int32_t createUISymbol = pure_getsym(createUIF.c_str());
        if (createUISymbol != 0 ) {
            createUI.reset(pure_new(pure_symbol(createUISymbol)));
            pure_interp_compile(interp.get(), createUISymbol);
            initUI();
        }
        
        // inject PureLink instance
        int32_t pureLinkInstanceSymbol = pure_sym(pureLinkVar.c_str());
        if (pureLinkInstanceSymbol) {
            pureLinkBinding.reset(pure_new(pure_pointer((void*) this)));
            pure_let(pureLinkInstanceSymbol, pureLinkBinding.get());
        }
        
        // inject MessageBus instance
        /*
        int32_t messageBusInstanceSymbol = pure_sym(messageBusVar.c_str());
        if (messageBusInstanceSymbol) {
            pure_expr* pointer = pure_new(pure_pointer((void*) bus));
            pure_let(messageBusInstanceSymbol, pointer);
        }
        */
        
    } else {
        if (debug) printf("Errors occured while evaluating script\n");
        pure_expr* pureerr = lasterrpos();
        const char* errstr = str(pureerr);
        if (debug) printf("%s\n", errstr);
        
        errors = strdup(errstr);
        clear_lasterr();
    }
    
    // unlock main interp
    pure_unlock_interp(local);
}
        
const std::string PureLink::getState() {
    if (debug) printf("OnGetState: <PureLink %p>\n", this);
    
    std::lock_guard<std::mutex> lock(mutex);
    pure_interp* local = pure_lock_interp(interp.get());
    
    std::string state = "";
    

        
    pure_expr* saveState = nullptr;
    int32_t saveStateFSymbol = pure_getsym(saveStateF.c_str());
    if (saveStateFSymbol != 0 ) {
        saveState = pure_symbol(saveStateFSymbol);
    }
    
    if(saveState == nullptr) {
        pure_unlock_interp(local);
        return state;
    }
    
    pure_expr* exceptions = nullptr;
    
    pure_expr* stateex = pure_appxv(saveState, 0, nullptr, &exceptions);
    if (exceptions) {
        report("getState", exceptions);
        pure_unlock_interp(local);
        return state;
    }
    
    state = str(stateex);
    if (debug) printf("getState: %s\n", str(stateex));
    pure_unlock_interp(local);
        
    return state;
    
}
        
void PureLink::setState(const std::string& state) {
    if (debug) printf("OnSetState: <PureLink %p>\n", this);
    if (debug) printf("Juce State sent: %s\n", state.c_str());
    
    std::lock_guard<std::mutex> lock(mutex);
    pure_interp* local = pure_lock_interp(interp.get());
    
    pure_expr* stateex = pure_eval(state.c_str());
    bool stateRestored = true;
    if (stateex) {
        if (debug) printf("setState: %s\n", str(stateex));
        pure_expr* restoreState = nullptr;
        int32_t restoreStateFSymbol = pure_getsym(restoreStateF.c_str());
        if (restoreStateFSymbol != 0 ) {
            restoreState = pure_symbol(restoreStateFSymbol);
            if (restoreState) {
                pure_expr* args[] = { stateex };
                pure_expr* exceptions = nullptr;
                pure_appxv(restoreState, 1, args, &exceptions);
                if (exceptions) {
                    report("setState", exceptions);
                    stateRestored = false; // prevent other processing below
                }
            }
        }
        //pure_free(stateex);
    }
    
    if (! stateRestored) {
        pure_unlock_interp(local);  // remember to unlock
        return; // no need to continue, since the state is wrong
    }

    
    
    // After the pure script has restored its state, we need to sync our state
    // so that when the editor restores its state, we provide the correct values
    // see: MidiManagerAudioProcessor::restoreEditorState()
    
    for (auto pair : widgetMap) {
        WidgetAction* wa = pair.second;
        UIWidget* w = wa->widget();
        pure_expr* pureWidget = wa->pureWidget();
        if (w->isValuePropertyInt()) {
            w->setCurrentIntValue(widgetIntProperty(pureWidget, valueProperty));
        } else if (w->isValuePropertyString()) {
           w->setCurrentStringValue(widgetStringProperty(pureWidget, valueProperty));
        }
    }
    
    pure_unlock_interp(local);

}
        
        
void PureLink::scriptEditorLoaded() {
    if (debug) printf("scriptEditorLoaded: <PureLink %p>\n", this);
    
    std::lock_guard<std::mutex> lock(mutex);
    pure_interp* local = pure_lock_interp(interp.get());
    
    int32_t onEditorLoadedFSymbol = pure_getsym(onEditorLoadedF.c_str());
    pure_expr* onEditorLoadedEx;
    if (onEditorLoadedFSymbol != 0 ) {
        onEditorLoadedEx = pure_symbol(onEditorLoadedFSymbol);
        if (onEditorLoadedEx) {
            pure_expr* exceptions = nullptr;
            pure_appxv(onEditorLoadedEx, 0, nullptr, &exceptions);
            if (exceptions) {
                report("scriptEditorLoaded", exceptions);
            }
        }
    }
    pure_unlock_interp(local);
}
void PureLink::initUI() {
    pure_expr* exceptions = nullptr;
    
    pure_expr* widgets = pure_appxv(createUI.get(), 0, nullptr, &exceptions);
    if (exceptions) {
        report("initUI", exceptions);
        return;
    }
    
    int32_t allWidgetsSymbol = pure_getsym(allWidgetsF.c_str());
    pure_expr* allWidgets = nullptr;
    if (allWidgetsSymbol) {
        allWidgets = pure_new(pure_symbol(allWidgetsSymbol));
        pure_expr* args[] = {widgets};
        pure_expr* validationResult = pure_appxv(allWidgets, 1, args, &exceptions);
        if (exceptions) {
            report("initUI", exceptions);
            return;
        }
        int validation = -1;
        if (!pure_is_int(validationResult, &validation) || validation == 0) {
            std::string errm = "Could not validate widgets returned by " + createUIF + " call";
            report(errm);
            return;
        }
        
        if (debug) printf("widget validation passed\n");
        
    } else {
        std::string errm = "Could not find " + createUIF + " function, UI will not be available";
        report(errm);
        return;
    }
    
    std::string m = str(widgets);
    log("Widgets", m);
    
    // process UI
    
    size_t size;
    pure_expr** elems;
    pure_is_listv(widgets, &size, &elems);  // we're sure it is a list, we just validated that
    if (size == 0 ) return;  // nothing to do here

    
    for (int i = 0; i < size; i++) {
        pure_expr* pureWidget = elems[i];
        
        int type_tag = widgetIntProperty(pureWidget, typeTagProperty);
        pure_expr* action = widgetExprProperty(pureWidget, actionProperty);
        switch (uiWidgetType(type_tag)) {
            case UIWidgetType::Slider: {
                UIWidgetSlider* control = new UIWidgetSlider {};
                control->code = widgetIntProperty(pureWidget, codeProperty);
                control->name = widgetStringProperty(pureWidget, nameProperty);
                control->value = widgetIntProperty(pureWidget, valueProperty);
                control->minimum = widgetIntProperty(pureWidget, "ui::min");
                control->maximum = widgetIntProperty(pureWidget, "ui::max");
                WidgetAction* widgetAction = new WidgetAction { control, pureWidget, action };
                widgetMap.insert(std::map<int, WidgetAction*>::value_type(control->code, widgetAction));
            }
                break;
                
            case UIWidgetType::Segmented: {
                UIWidgetSegmented* control = new UIWidgetSegmented {};
                control->code = widgetIntProperty(pureWidget, codeProperty);
                control->name = widgetStringProperty(pureWidget, nameProperty);
                control->value = widgetIntProperty(pureWidget, valueProperty);
                control->labels = widgetStringListProperty(pureWidget, "ui::labels");
                WidgetAction* widgetAction = new WidgetAction { control, pureWidget, action };
                widgetMap.insert(std::map<int, WidgetAction*>::value_type(control->code, widgetAction));
            }
                break;
                
            case UIWidgetType::PopUp: {
                UIWidgetPopUp* control = new UIWidgetPopUp {};
                control->code = widgetIntProperty(pureWidget, codeProperty);
                control->name = widgetStringProperty(pureWidget, nameProperty);
                control->value = widgetIntProperty(pureWidget, valueProperty);
                control->labels = widgetStringListProperty(pureWidget, "ui::labels");
                WidgetAction* widgetAction = new WidgetAction { control, pureWidget, action };
                widgetMap.insert(std::map<int, WidgetAction*>::value_type(control->code, widgetAction));
            }
                break;
                
            case UIWidgetType::CheckBox: {
                UIWidgetCheckBox* control = new UIWidgetCheckBox {};
                control->code = widgetIntProperty(pureWidget, codeProperty);
                control->name = widgetStringProperty(pureWidget, nameProperty);
                control->value = widgetIntProperty(pureWidget, valueProperty);
                WidgetAction* widgetAction = new WidgetAction { control, pureWidget, action };
                widgetMap.insert(std::map<int, WidgetAction*>::value_type(control->code, widgetAction));
            }
                break;
            
            case UIWidgetType::Label: {
                UIWidgetLabel* control = new UIWidgetLabel {};
                control->code = widgetIntProperty(pureWidget, codeProperty);
                control->name = widgetStringProperty(pureWidget, nameProperty);
                control->value = widgetStringProperty(pureWidget, valueProperty);
                WidgetAction* widgetAction = new WidgetAction { control, pureWidget, action };
                widgetMap.insert(std::map<int, WidgetAction*>::value_type(control->code, widgetAction));
            }
                break;
                
            case UIWidgetType::Unkown: {
                log("Widget Warning", "Unsupported widget type");
            }
                break;
        }
    }
    
}
        
        
void PureLink::signal(int widgetCode, int newValue) {
    WidgetAction* wa = widgetMap[widgetCode];
    if (debug) printf("Widget name: %s\n", wa->widget()->name.c_str());
    Event e;
    e.uiEvent = UIEvent::UISignal;
    e.Change.code = widgetCode;
    e.Change.intValue = newValue;
    wa->widget()->setCurrentIntValue(newValue);
    bus->publishAsync(e);
}
        
        
void PureLink::signal(int widgetCode, const std::string& newValue) {
    WidgetAction* wa = widgetMap[widgetCode];
    if (debug) printf("Widget name: %s\n", wa->widget()->name.c_str());
    Event e;
    e.uiEvent = UIEvent::UISignal;
    e.Change.code = widgetCode;
    e.Change.stringValue = newValue;
    wa->widget()->setCurrentStringValue(newValue);
    bus->publishAsync(e);
}

        
        
bool PureLink::isWidget(const pure_expr* possibleWidget) {

    bool result = false;
    int32_t isWidgetSymbol = pure_getsym(isWidgetF.c_str());
    pure_expr* isWidget = nullptr;
    if (isWidgetSymbol) {
        isWidget = pure_new(pure_symbol(isWidgetSymbol));
        pure_expr* args[] = {(pure_expr*) possibleWidget};
        pure_expr* exceptions = nullptr;
        pure_expr* validationResult = pure_appxv(isWidget, 1, args, &exceptions);
        if (exceptions) {
            report("isWidget", exceptions);
            result = false;
        } else {
            int validation;
            if (pure_is_int(validationResult, &validation) == 0 || validation == 0) {
                std::string errm = "Could not validate widgets returned by " + isWidgetF + " call";
                report(errm);
                result = false;
            }
        }
        
        result = true;
        
        if (debug) printf("widget validation passed\n");
        
    } else {
        std::string errm = "Could not find function: " + isWidgetF;
        report(errm);
        result = false;
    }
    

    return result;
}
   
pure_expr* PureLink::widgetExprProperty(const pure_expr* widget, const std::string& name) {
    int32_t propertySymbol = pure_getsym(name.c_str());
    pure_expr* propertyFunction = nullptr;

    pure_expr* value = nullptr;
    if (propertySymbol) {
        propertyFunction = pure_symbol(propertySymbol);
        pure_expr* args[] = { (pure_expr*) widget};
        pure_expr* exception = nullptr;
        value = pure_appxv(propertyFunction, 1, args, &exception);
        if (exception) {
            std::ostringstream input , output;
            input << "Error" << std::endl;
            output << "Could not read property: " << name << " on widget: " << str(widget) << std::endl;
            output << "Pure Exception: " << str(exception) << std::endl << std::endl;
            log(input.str(), output.str());
        }
        
    }
    return value;
}

        
int PureLink::widgetIntProperty(const pure_expr* widget, const std::string& name) {
    int32_t propertySymbol = pure_getsym(name.c_str());
    pure_expr* propertyFunction = nullptr;
    int value = 0;
    if (propertySymbol) {
        propertyFunction = pure_symbol(propertySymbol);
        pure_expr* args[] = { (pure_expr*) widget};
        pure_expr* exception = nullptr;
        pure_expr* result = pure_appxv(propertyFunction, 1, args, &exception);
        if (exception) {
            std::ostringstream input , output;
            input << "Error" << std::endl;
            output << "Could not read property: " << name << " on widget: " << str(widget) << std::endl;
            output << "Pure Exception: " << str(exception) << std::endl;
            if (debug) {
                output << pure_evalcmd("bt") << std::endl;
            }
            output << std::endl;
            log(input.str(), output.str());
            return value;
        }
        if (!pure_is_int(result, &value)) {
            std::ostringstream input , output;
            input << "Warning" << std::endl;
            output << "Could not read property: " << name << " on widget: " << str(widget) << std::endl << std::endl;
            log(input.str(), output.str());
        }
    }
    return value;
}
        
std::string PureLink::widgetStringProperty(const pure_expr* widget, const std::string& name) {
    int32_t propertySymbol = pure_getsym(name.c_str());
    pure_expr* propertyFunction = nullptr;
    std::string value = "";
    if (propertySymbol) {
        propertyFunction = pure_symbol(propertySymbol);
        pure_expr* args[] = { (pure_expr*) widget};
        pure_expr* exception = nullptr;
        pure_expr* result = pure_appxv(propertyFunction, 1, args, &exception);
        if (exception) {
            std::ostringstream input , output;
            input << "Error" << std::endl;
            output << "Could not read property: " << name << " on widget: " << str(widget) << std::endl;
            output << "Pure Exception: " << str(exception) << std::endl;
            if (debug) {
                output << pure_evalcmd("bt") << std::endl;
            }
            output << std::endl;
            log(input.str(), output.str());
            return value;
        }
        char* buf;
        if (!pure_is_string_dup(result, &buf)) {
            std::ostringstream input , output;
            input << "Warning" << std::endl;
            output << "Could not read property: " << name << " on widget: " << str(widget) << std::endl << std::endl;
            log(input.str(), output.str());
        } else {
            value = std::string(buf);
        }
    }

    return value;
}
        
std::vector<std::string> PureLink::widgetStringListProperty(const pure_expr* widget, const std::string& name) {
    int32_t propertySymbol = pure_getsym(name.c_str());
    pure_expr* propertyFunction = nullptr;
    std::vector<std::string> value ;
    if (propertySymbol) {
        propertyFunction = pure_symbol(propertySymbol);
        pure_expr* args[] = { (pure_expr*) widget};
        pure_expr* exception = nullptr;
        pure_expr* result = pure_appxv(propertyFunction, 1, args, &exception);
        pure_expr** elems;
        size_t size;
        if (exception) {
            std::ostringstream input , output;
            input << "Error" << std::endl;
            output << "Could not read property: " << name << " on widget: " << str(widget) << std::endl;
            output << "Pure Exception: " << str(exception) << std::endl;
            if (debug) {
                output << pure_evalcmd("bt") << std::endl;
            }
            output << std::endl;
            log(input.str(), output.str());
            return value;
        }
        if (!pure_is_listv(result, &size, &elems)) {
            std::ostringstream input , output;
            input << "Warning" << std::endl;
            output << "Could not read property: " << name << " on widget: " << str(widget) << std::endl << std::endl;
            log(input.str(), output.str());
        } else {
            for (int i = 0; i < size; i++) {
                char* buf;
                pure_is_string_dup(elems[i], &buf);
                value.push_back(std::string(buf));
            }
        }
    }
    
    return value;
}
        
void PureLink::onEvent(const Event& event) {
    switch (event.uiEvent) {
        case UIEvent::EditorConfigured:
            this->logging = event.Configuration.debug;
            this->silenceOnErrors = event.Configuration.silenceOnErrors;
            break;
        case UIEvent::EditorWidgetChanged:
            onEditorWidgetChange(event);
            break;
        default:
            break;
    }
}


void PureLink::onEditorWidgetChange(const Event& event) {
    std::lock_guard<std::mutex> lock(mutex);
    pure_interp* local = pure_lock_interp(interp.get());
    
    WidgetAction* wa = widgetMap[event.Change.code]; // can this fail?
    
    pure_expr* action = wa->action();
    pure_expr* newValue;
    if (wa->widget()->isValuePropertyInt()) {
        newValue = pure_int(event.Change.intValue);
        wa->widget()->setCurrentIntValue(event.Change.intValue);
    } else if (wa->widget()->isValuePropertyString()) {
        newValue = pure_cstring_dup(event.Change.stringValue.c_str());
        wa->widget()->setCurrentStringValue(event.Change.stringValue);
    }
    
    
    
    // call ui::set_value then call the action
    
    pure_expr* setValueArgs[] = { wa->pureWidget(), newValue };
    pure_expr* exception = nullptr;
    int32_t setValueFSymbol = pure_getsym(setValueF.c_str());
    pure_expr* sve = pure_symbol(setValueFSymbol);
    if (sve) {
        pure_appxv(sve, 2, setValueArgs, &exception);
        if (exception) {
            report("onEditorWidgetChange", exception);
        }
    }
    
    pure_expr* actionArgs[] = { newValue };
    pure_appxv(action, 1, actionArgs, &exception);
    
    if (exception) {
        report("onEditorWidgetChange", exception);
    }
    
    pure_unlock_interp(local);
    
}

void PureLink::callPureFinalize() {
    pure_finalize();
}


bool PureLink::hasErrors() {
    //std::lock_guard<std::mutex> lock(mutex);
    return !errors.empty();
}

const std::string PureLink::getFilename() {
    //std::lock_guard<std::mutex> lock(mutex);
    return filename;
}
        
        
const std::vector<UIWidget*> PureLink::getWidgets() {
    std::vector<UIWidget*> widgets;
    for(auto& e : widgetMap) {
        widgets.push_back(e.second->widget());
    }
    return widgets;
}

const std::string PureLink::getErrors() {
    //std::lock_guard<std::mutex> lock(mutex);
    return errors;
}

        
void PureLink::log(const std::string& ingoing, const std::string& outgoing) {
    
    std::ostringstream input, output;
    input << "[In] " << ingoing << std::endl;
    output << "[Out] " << outgoing << std::endl << std::endl;
    Event e;
    e.uiEvent = UIEvent::Logging;
    e.Logging.input = input.str();
    e.Logging.output = output.str();
    bus->publishAsync(e);
    
}
        
void PureLink::report(const std::string& who, pure_expr* exception) {
    std::ostringstream m;
    
    m << "An exception ("
      << str(exception)
      << ") has been thrown while executing your script, please debug your code";
    
    if (debug) {
        //m << std::endl << pure_evalcmd("bt") << std::endl;
        printf("reported from: %s, exception: %s\n", who.c_str(), str(exception));
    }
    
    errors = m.str();
    Event e;
    e.uiEvent = UIEvent::ScriptErrors;
    e.ScriptData.compilationErrors = errors;
    
    bus->publish(e);
}
        
void PureLink::report(std::string& message) {
    errors = message;
    Event e;
    e.uiEvent = UIEvent::ScriptErrors;
    e.ScriptData.compilationErrors = errors;
    
    bus->publish(e);
}
        


pure_expr* PureLink::createNoteOnMessage(int channel, int note, int velocity, int position) {
    //if (debug) printf("PureLink: createNoteOnMessage\n");
    //pure_switch_interp(interp);
    pure_expr* cx = pure_int(channel);
    pure_expr* nx = pure_int(note);
    pure_expr* vx = pure_int(velocity);
    pure_expr* px = pure_int(position);
    int32_t noteMessageSymbol = pure_sym("noteOn");
    pure_expr* result = nullptr;
    if (noteMessageSymbol) {
        pure_expr* function = pure_symbol(noteMessageSymbol);
        pure_expr* args[] = {cx,nx,vx,px};
        pure_expr* exception = nullptr;
        result = pure_appxv(function, 4, args, &exception);
        if (exception) {
            report("createNoteOnMessage", exception);
        }
    }
    
    return result;
}

pure_expr* PureLink::createNoteOffMessage(int channel, int note, int position) {
    //if (debug) printf("PureLink: createNoteOffMessage\n");
    //pure_switch_interp(interp);
    pure_expr* cx = pure_int(channel);
    pure_expr* nx = pure_int(note);
    pure_expr* px = pure_int(position);
    int32_t noteMessageSymbol = pure_sym("noteOff");
    pure_expr* result = nullptr;
    if (noteMessageSymbol) {
        pure_expr* function = pure_symbol(noteMessageSymbol);
        pure_expr* args[] = {cx,nx,px};
        pure_expr* exception = nullptr;
        result = pure_appxv(function, 3, args, &exception);
        if (exception) {
            report("createNoteOffMessage", exception);
        }
    }
    return result;
}

bool PureLink::createMessageFrom(pure_expr* expr, MidiMessage& message, int& position) {
    //if (debug) printf("PureLink: createMessageFrom\n");
    
    // The following reads a tuple on the form (word::int, position::int), where word is the midi message
    //pure_switch_interp(interp);
    size_t argc;
    pure_expr** args;
    int32_t word;
    
    bool success = false;
    
    if (pure_is_tuplev(expr, &argc, &args)) {
        if (argc == 2 && pure_is_int(args[0], &word) && pure_is_int(args[1], &position)) {
            uint8 status = word & 0xff;
            uint8 byte1  = (word >> 8 ) & 0xff;
            uint8 byte2  = (word >> 16) & 0xff;
            message = MidiMessage(status, byte1, byte2);
            success = true;
        }
    } else {
        std::string errm =
        "Your script is not returning the supported format.\n"
        "process_midi_buffer should return a list of tuples, each tuple (e,p) represents a MIDI event 'e' and position 'p'\n";
        report(errm);
    }
    
    return success;
}

MidiBuffer PureLink::processBlock(MidiBuffer& input) {
    if (hasErrors()) {
        return silenceOnErrors? MidiBuffer() : input;
    }
    
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!errors.empty()) return silenceOnErrors? MidiBuffer() : input;

    pure_interp* local = pure_lock_interp(interp.get());
    
    //if (debug) printf("PureLink: processBlock\n");
    
    MidiBuffer output;
    MidiBuffer::Iterator inputItr(input);
    MidiMessage message;
    int position;
    std::vector<pure_expr*> v;
    while (inputItr.getNextEvent(message, position)) {
        
        int size = message.getRawDataSize();
        
        if (size > 0) {
            const uint8* data = message.getRawData();
            uint8 status = data[0];
            uint8 data1  = size > 1 ? data[1]: 0;
            uint8 data2  = size > 2 ? data[2]: 0;
            int32_t word = ((data2<<16) & 0xff0000) | ((data1<<8) & 0xff00) | (status & 0xff);
            int32_t tuplevalues[] = {word, position};
            pure_expr* tupleexpr = pure_inttuplev(2, tuplevalues);
            if (logging) {
                std::ostringstream in, out;
                in << "word: " << word << ", status: " << (int)status << ", data1: " << (int)data1 << ", data2: " << (int)data2;
                out << "tuple: " << str(tupleexpr);
                log(in.str(), out.str());
            }
            v.push_back(tupleexpr);
        }
    }
    
    if (!v.empty()) {
        pure_expr** listargs = &v[0];
        //pure_expr* tail = pure_listl(0);
        pure_expr* list = pure_listv(v.size(), listargs);
        pure_expr* args[] = {list};
        if (processMidiBuffer) {
            pure_expr* exceptions = nullptr;
            pure_expr* result = pure_appxv(processMidiBuffer.get(), 1, args, &exceptions);
            if (exceptions) {
                report("processBlock", exceptions);
            } else {
                
                if (logging) {
                    char* ingoing = str(list);
                    char* outgoing = str(result);
                    log(ingoing, outgoing);
                }
                
                pure_free(list);
                size_t resultSize;
                pure_expr** elems;
                bool isList = pure_is_listv(result, &resultSize, &elems);
                if (isList) {
                    for (size_t i = 0; i < resultSize ; i++) {
                        pure_expr* current = elems[i];
                        if (createMessageFrom(current, message, position)) {
                            output.addEvent(message, position);
                        }
                        
                    }
                } else {
                    std::string errm =
                    "Your script is not returning the supported format.\n"
                    "process_midi_buffer should return a list of tuples, each tuple (e,p) represents a MIDI event 'e' and position 'p'\n";
                    report(errm);
                }
            }
        }
    }
    
    pure_unlock_interp(local);
    return output;
}
