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
#include <mutex>

class NativeEditorControllerListener;

@interface NativeEditorController : NSViewController {
    NSString* loadedFileName;
    NSAttributedString* errors;
    BOOL debug;
    BOOL silenceOnErrors;
    NativeEditorControllerListener* listener;
    std::mutex mutex;
}


@property (copy) NSString* loadedFileName;
@property (copy) NSAttributedString* errors;
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
        printf("NativeEditorControllerListener: %p\n", this);
        bus->addListener(this);
    }
    ~NativeEditorControllerListener() {
        bus->removeListener(this);
    }
    void onEvent(const Event& event) {
        [controller onEvent: event];
    }
};


