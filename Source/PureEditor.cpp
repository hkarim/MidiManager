//
//  PureNative.cpp
//  MidiManager
//
//  Created by Hossam Karim on 8/31/14.
//
//
#include "../JuceLibraryCode/JuceHeader.h"
#include <pure/runtime.h>
#include "PureEditor.h"




extern "C" {
    void setWidth(pure_expr* expr, int w) {
        //printf("pure is setting the editor width: %d\n", w);
        void* p;
        if (pure_is_pointer(expr, &p)) {
            if (p) {
                PureEditor* editor = (PureEditor*) p;
                editor->width = w;
                if (editor->listener) editor->listener->editorUpdated(editor);
            }
        }
    }
    void setHeight(pure_expr* expr, int h) {
        //printf("pure is setting the editor height: %d\n", h);
        void* p;
        if (pure_is_pointer(expr, &p)) {
            if (p) {
                PureEditor* editor = (PureEditor*) p;
                editor->height = h;
                if (editor->listener) editor->listener->editorUpdated(editor);
            }
        }
    }
}
