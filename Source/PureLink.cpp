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
#include <sys/stat.h>



struct MainInterpreter {
    
    pure_interp* createInterpreter() {
        const char* currentEx =
          File::getSpecialLocation(File::SpecialLocationType::currentExecutableFile).getFullPathName().toRawUTF8();
        std::string location =
          File::getSpecialLocation(File::SpecialLocationType::currentApplicationFile).getFullPathName().toStdString();
        std::string includeDir =
          std::string(location).append("/Contents/Resources/");
        std::string libDir =
          std::string(location).append("/Contents/MacOS/");
        
        
        char* argv[] = {
            strdup(currentEx),
            strdup("--eager-jit"),
            strdup("-I"),
            strdup(includeDir.c_str()),
            strdup("-L"),
            strdup(libDir.c_str()),
            nullptr
        };
        //printf("creating new interpreter instance\n");
        pure_interp* local = pure_lock_interp(defaultInstance);
        pure_interp* newInstance = pure_create_interp(6, argv);
        pure_unlock_interp(local);
        return newInstance;
    }
    
    pure_interp* defaultInstance = pure_create_interp(0, 0);
} interpreter;

inline bool exists (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

PureLink::PureLink(const std::string& filename, MessageBus* bus) :
EventListener { }, bus { bus }, filename { filename } {
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
    if (processMidiBuffer) pure_free(processMidiBuffer);
    if (createEditor) pure_free(createEditor);
    if (block) pure_free(block);
    if (editor) delete editor;
    if (editorExpr) pure_free(editorExpr);
    pure_interp* defaultInstance = interpreter.defaultInstance;
    if (interp) {
        pure_interp* local = pure_lock_interp(defaultInstance);
        pure_delete_interp(interp);
        pure_unlock_interp(local);
    }
}

void PureLink::init() {
    if (debug) printf("PureLink: init\n");
    if (code.empty()) {
        return;
    }

    pure_interp* local;
    interp = interpreter.createInterpreter();
    local = pure_lock_interp(interp);
    if (debug) printf("Pure Interpreter: %p\n", (void*) interp);
    pure_expr* evaluated = pure_eval(code.c_str());
    if (evaluated) {
        block = pure_new(evaluated);
        pure_interp_compile(interp, 0);
        
        int32_t processMidiBufferSymbol = pure_getsym("processMidiBuffer");
        if (processMidiBufferSymbol != 0 ) {
            processMidiBuffer = pure_new(pure_symbol(processMidiBufferSymbol));
        }
        
        int32_t createEditorSymbol = pure_getsym("createEditor");
        if (createEditorSymbol != 0 ) {
            createEditor = pure_new(pure_symbol(createEditorSymbol));
        }
        
    } else {
        if (debug) printf("Errors occured while evaluating script\n");
        pure_expr* pureerr = lasterrpos();
        const char* errstr = str(pureerr);
        pure_free(pureerr);
        if (debug) printf("%s\n", errstr);
        
        errors = strdup(errstr);
        clear_lasterr();
    }
    pure_unlock_interp(local);
}
        
void PureLink::onEvent(const Event& event) {
    switch (event.uiEvent) {
        case UIEvent::EditorConfigured:
            this->logging = event.Configuration.debug;
            this->silenceOnErrors = event.Configuration.silenceOnErrors;
            break;
        default:
            break;
    }
}

void PureLink::callPureFinalize() {
    pure_finalize();
}

PureEditor* PureLink::getPureEditor() {
    //std::lock_guard<std::mutex> lock(mutex);
    return editor;
}

bool PureLink::hasErrors() {
    //std::lock_guard<std::mutex> lock(mutex);
    return !errors.empty();
}

const std::string PureLink::getFilename() {
    //std::lock_guard<std::mutex> lock(mutex);
    return filename;
}

const std::string PureLink::getErrors() {
    //std::lock_guard<std::mutex> lock(mutex);
    return errors;
}

void PureLink::addGuiHook() {
    if (createEditor == nullptr)
        return;
    editor = new PureEditor();
    editorExpr = pure_pointer(editor);
    pure_expr* args[] = {editorExpr};
    pure_appv(createEditor, 1, args);
}
        
void PureLink::log(const std::string& ingoing, const std::string& outgoing) {
    
    std::ostringstream input, output;
    input << "[In] " << ingoing << std::endl;
    output << "[Out] " << outgoing << std::endl << std::endl;
    Event e;
    e.uiEvent = UIEvent::Logging;
    e.MidiStream.input = input.str();
    e.MidiStream.output = output.str();
    bus->publishAsync(e);
    
}

pure_expr* PureLink::createNoteOnMessage(int channel, int note, int velocity, int position) {
    if (debug) printf("PureLink: createNoteOnMessage\n");
    //pure_switch_interp(interp);
    pure_expr* cx = pure_int(channel);
    pure_expr* nx = pure_int(note);
    pure_expr* vx = pure_int(velocity);
    pure_expr* px = pure_int(position);
    int32_t noteMessageSymbol = pure_sym("noteOn");
    pure_expr* result = nullptr;
    if (noteMessageSymbol) {
        try {
            pure_expr* function = pure_symbol(noteMessageSymbol);
            pure_expr* args[] = {cx,nx,vx,px};
            result = pure_appv(function, 4, args);
        } catch(...) {
            printf("PureLink: createNoteOnMessage Exception\n");
            //const char* e = lasterr();
            //AlertWindow::showNativeDialogBox("Error", String(e), false);
        }
        
    }
    
    return result;
}

pure_expr* PureLink::createNoteOffMessage(int channel, int note, int position) {
    if (debug) printf("PureLink: createNoteOffMessage\n");
    //pure_switch_interp(interp);
    pure_expr* cx = pure_int(channel);
    pure_expr* nx = pure_int(note);
    pure_expr* px = pure_int(position);
    int32_t noteMessageSymbol = pure_sym("noteOff");
    pure_expr* result = nullptr;
    if (noteMessageSymbol) {
        try {
            pure_expr* function = pure_symbol(noteMessageSymbol);
            pure_expr* args[] = {cx,nx,px};
            result = pure_appv(function, 3, args);
        } catch(...) {
            const char* e = lasterr();
            AlertWindow::showNativeDialogBox("Error", String(e), false);
        }
        
    }
    return result;
}

bool PureLink::createMessageFrom(pure_expr* expr, MidiMessage& message, int& position) {
    if (debug) printf("PureLink: createMessageFrom\n");
    
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
        errors =
        "Your script is not returning the supported format.\n"
        "processMidiBuffer should return a list of tuples, each tuple (e,p) represents a MIDI event 'e' and position 'p'\n";
        Event e;
        e.uiEvent = UIEvent::ScriptErrors;
        e.ScriptData.compilationErrors = errors;
        bus->publish(e);
    }
    
    return success;
}

MidiBuffer PureLink::processBlock(MidiBuffer& input) {
    if (hasErrors()) {
        return silenceOnErrors? MidiBuffer() : input;
    }
    
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!errors.empty()) return silenceOnErrors? MidiBuffer() : input;

    pure_interp* local = pure_lock_interp(interp);
    
    if (debug) printf("PureLink: processBlock\n");
    
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
        pure_expr* tail = pure_listl(0);
        pure_expr* list = pure_listv2(v.size(), listargs, tail);
        pure_expr* args[] = {list};
        if (processMidiBuffer) {
            pure_expr* exceptions = nullptr;
            pure_expr* result = pure_appxv(processMidiBuffer, 1, args, &exceptions);
            if (exceptions) {
                errors =
                  "An exception (" +
                  std::string(str(exceptions)) +
                  ") has been thrown while executing your script, please debug your code";
                Event e;
                e.uiEvent = UIEvent::ScriptErrors;
                e.ScriptData.compilationErrors = errors;
                
                bus->publish(e);
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
                    errors =
                    "Your script is not returning the supported format.\n"
                    "processMidiBuffer should return a list of tuples, each tuple (e,p) represents a MIDI event 'e' and position 'p'\n";
                    Event e;
                    e.uiEvent = UIEvent::ScriptErrors;
                    e.ScriptData.compilationErrors = errors;
                    bus->publish(e);
                }
            }
        }
    }
    
    pure_unlock_interp(local);
    return output;
}
