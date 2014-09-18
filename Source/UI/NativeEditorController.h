//
//  NativeEditorController.h
//  MidiManager
//
//  Created by Hossam Karim on 9/2/14.
//
//

#import <Cocoa/Cocoa.h>
#import "CustomControls.h"
#include "MessageBus.h"
#include <map>

class NativeEditorControllerListener;

@interface NativeEditorController : NSViewController {
    NSString* loadedFileName;
    NSString* errors;
    BOOL debug;
    BOOL silenceOnErrors;
    NativeEditorControllerListener* listener;
}


@property (copy) NSString* loadedFileName;
@property (copy) NSString* errors;
@property BOOL debug;
@property BOOL silenceOnErrors;


-(void) setMessageBus: (MessageBus*) encapsulatedMessageBus;

-(void) editorLoaded;

-(void) log:(NSString*) input withOutput:(NSString*) output;

-(void) onEvent:(const Event&) event;

-(NSArray *) openFiles;

@end


class NativeEditorControllerListener : public EventListener {
private:
    NativeEditorController* controller;
    MessageBus* bus;
public:
    NativeEditorControllerListener(NativeEditorController* controller, MessageBus* bus) :
    EventListener {}, controller { controller }, bus { bus } {
        bus->addListener(this);
    }
    ~NativeEditorControllerListener() {
        bus->removeListener(this);
    }
    void onEvent(const Event& event) {
        [controller onEvent: event];
    }
};


