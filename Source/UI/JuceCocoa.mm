//
//  JuceCocoa.m
//  MidiManager
//
//  Created by Hossam Karim on 9/2/14.
//
//

#import <Cocoa/Cocoa.h>
#include "JuceCocoa.h"
#include "NativeEditorController.h"


JuceCocoa::JuceCocoa(MessageBus* bus) : EventListener(), bus(bus) {
    bus->addListener(this);
}

JuceCocoa::~JuceCocoa() {
    bus->removeListener(this);
}

void* JuceCocoa::createNSView() {
    
    String currentEx = File::getSpecialLocation(File::SpecialLocationType::currentApplicationFile).getFullPathName();

    const char* cpath = currentEx.toRawUTF8();
    std::string s(cpath);
    //NSLog(@"cstring: %s", s.c_str());
    NSString* path = [NSString stringWithFormat:@"%s", s.c_str()];
    //NSLog(@"NSString path = %@", path);
    NSBundle* bundle = [[NSBundle alloc] initWithPath:path];
    //NSLog(@"Bundle path %@", [bundle bundlePath]);
    NativeEditorController* controller =
      [[NativeEditorController alloc] initWithNibName:@"NativeEditorView" bundle:bundle];
    //[controller autorelease];
    
    this->controller = (void*) controller;
    [controller setMessageBus: bus];
    
    NSView *view = [controller view];
    this->view = (void*) view;
    [controller editorLoaded];
    

    return view;
}


void JuceCocoa::onEvent(const Event& event) {
    switch (event.uiEvent) {
        case UIEvent::ScriptErrors:
            if (controller) {
                //NSLog(@"ScriptErrors: %s", event.ScriptData.compilationErrors);
                NativeEditorController* c = (NativeEditorController*) controller;
                [c setErrors:[NSString stringWithFormat:@"%s", event.ScriptData.compilationErrors.c_str()]];
            }
            break;
        case UIEvent::EditorRestoreState:
            if (controller) {
                //NSLog(@"FileChosen: %s", event.FileData.filename);
                NativeEditorController* c = (NativeEditorController*) controller;
                [c setLoadedFileName:[NSString stringWithFormat:@"%s", event.FileData.filename.c_str()]];
                if (!event.ScriptData.compilationErrors.empty()) {
                    [c setErrors:[NSString stringWithFormat:@"%s", event.ScriptData.compilationErrors.c_str()]];
                } else {
                    [c setErrors:@"Compiled Successfully"];
                }
                
            }
            break;
            
        default:
            break;
    }
}














