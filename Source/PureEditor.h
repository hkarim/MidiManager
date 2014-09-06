//
//  PureNative.h
//  MidiManager
//
//  Created by Hossam Karim on 9/1/14.
//
//

#ifndef MidiManager_PureNative_h
#define MidiManager_PureNative_h

class PureEditor;


class PureEditorListener {
public:
    virtual ~PureEditorListener() {}
    virtual void editorUpdated(PureEditor* pureEditor) = 0;
};

class PureEditor {
    
public:
    PureEditor() {}
    ~PureEditor() {
        if (listener) {
            delete listener;
        }
    }
    PureEditorListener* listener = nullptr;
    int width = 500;
    int height = 200;
    
} ;

#endif
