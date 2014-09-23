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
#include <memory>

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


-(void) setMessageBus: (std::shared_ptr<MessageBus>) encapsulatedMessageBus;

-(void) editorLoaded;

-(void) log:(NSString*) input withOutput:(NSString*) output;

-(void) onEvent:(const Event&) event;

-(NSArray *) openFiles;

@end


class NativeEditorControllerListener : public EventListener {
private:
    NativeEditorController* controller;
    std::shared_ptr<MessageBus> bus;
public:
    NativeEditorControllerListener(NativeEditorController* controller, std::shared_ptr<MessageBus> bus) :
    EventListener {}, controller { controller }, bus { bus } {
        printf("NativeEditorControllerListener: %p\n", this);
        bus->addListener(this);
    }
    ~NativeEditorControllerListener() {
        bus->removeListener(this);
        printf("NativeEditorControllerListener %p freed\n", this);
    }
    void onEvent(const Event& event) {
        [controller onEvent: event];
    }
};


