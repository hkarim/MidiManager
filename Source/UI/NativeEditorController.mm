//
//  NativeEditorController.m
//  MidiManager
//
//  Created by Hossam Karim on 9/2/14.
//
//

#import "NativeEditorController.h"

@interface NativeEditorController ()

@end

@implementation NativeEditorController

__weak MessageBus* bus;

@synthesize loadedFileName;
@synthesize errors;


-(void) setMessageBus: (MessageBus*) encapsulatedMessageBus {
    bus = encapsulatedMessageBus;
}


-(Event) initEvent {
    Event event {};
    NSRect rect = [[self view] frame];
    event.ViewCoordinates.w = rect.size.width;
    event.ViewCoordinates.h = rect.size.height;
    return event;
}

-(void) editorLoaded {
    //NSLog(@"NativeEditorController::editorLoaded %@", self);
    self.loadedFileName = @"Nothing Loaded right now";
    self.errors = @"No errors";
    
    Event e = [self initEvent];
    
    e.uiEvent = UIEvent::EditorLoaded;
    
    bus->publish(e);
}

- (void) ignoreme {
    NSAlert* alert = [[NSAlert alloc] init];
    [alert setMessageText:@"Hello"];
    [alert runModal];
}


- (IBAction)loadScriptClicked:(NSButton *)sender {
    NSArray* files = [self openFiles];
    if (files) {
        Event e = [self initEvent];
        e.uiEvent = UIEvent::FileChosen;
        
        NSURL* url = [files objectAtIndex:0];
        char* filename = strdup([[url path] UTF8String]);
        e.FileData.filename = filename;
        self.loadedFileName = [url path];
        
        bus->publish(e);
    }
    
}

- (NSArray *) openFiles {
    NSArray *fileTypes = [NSArray arrayWithObjects:@"pure",nil];
    NSOpenPanel * panel = [NSOpenPanel openPanel];
    [panel setAllowsMultipleSelection:NO];
    [panel setCanChooseDirectories:NO];
    [panel setCanChooseFiles:YES];
    [panel setFloatingPanel:YES];
    [panel setAllowedFileTypes:fileTypes];
    [panel setDirectoryURL:[NSURL URLWithString:NSHomeDirectory()]];
    NSInteger result = [panel runModal];
    if(result == NSOKButton) {
        return [panel URLs];
    }
    return nil;
}

@end
