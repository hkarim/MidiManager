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


JuceCocoa::JuceCocoa(std::shared_ptr<MessageBus> bus) : EventListener(), bus(bus) {
    printf("JuceCocoa: %p\n", this);
    bus->addListener(this);
}

JuceCocoa::~JuceCocoa() {
    bus->removeListener(this);
    printf("JuceCocoa %p freed\n", this);
}

void JuceCocoa::releaseController() {
    if (this->controller != nil) {
        NativeEditorController* c = (NativeEditorController*) this->controller;
        [c release];
    }
    
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

double JuceCocoa::currentHeight() {
    if (this->controller != nil) {
        NativeEditorController* c = (NativeEditorController*) this->controller;
        return [[c view] frame].size.height;
    }
    return 0.0f;
}

double JuceCocoa::currentWidth() {
    if (this->controller != nil) {
        NativeEditorController* c = (NativeEditorController*) this->controller;
        return [[c view] frame].size.width;
    }
    return 0.0f;
}


void JuceCocoa::onEvent(const Event& event) {
    
}














