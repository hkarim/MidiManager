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
}


@property (copy) NSString* loadedFileName;
@property (copy) NSString* errors;


-(void) setMessageBus: (MessageBus*) encapsulatedMessageBus;

-(void) editorLoaded;

-(NSArray *) openFiles;

@end


