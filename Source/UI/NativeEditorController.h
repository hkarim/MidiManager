//
//  NativeEditorController.h
//  MidiManager
//
//  Created by Hossam Karim on 9/2/14.
//
//

#import <Cocoa/Cocoa.h>
#include "MessageBus.h"

@interface NativeEditorController : NSViewController {
    NSString* loadedFileName;
    NSString* errors;
    BOOL debug;
    BOOL silenceOnErrors;
}


@property (copy) NSString* loadedFileName;
@property (copy) NSString* errors;
@property BOOL debug;
@property BOOL silenceOnErrors;


-(void) setMessageBus: (MessageBus*) encapsulatedMessageBus;

-(void) editorLoaded;

-(void) log:(NSString*) packet;



-(NSArray *) openFiles;

@end


