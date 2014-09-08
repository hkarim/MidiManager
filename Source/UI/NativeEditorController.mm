//
//  NativeEditorController.m
//  MidiManager
//
//  Created by Hossam Karim on 9/2/14.
//
//

#import "NativeEditorController.h"

@interface NativeEditorController ()
@property (assign) IBOutlet NSTextView *debugTextView;

@end

@implementation NativeEditorController

__weak MessageBus* bus;

@synthesize loadedFileName;
@synthesize errors;
@synthesize debug;
@synthesize silenceOnErrors;


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

-(void) log:(NSString*) packet {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSAttributedString* attr = [[NSAttributedString alloc] initWithString:packet];
        
        [[_debugTextView textStorage] appendAttributedString:attr];
        [_debugTextView scrollRangeToVisible:NSMakeRange([[_debugTextView string] length], 0)];
    });
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

- (IBAction)debugOnOff:(id)sender {
    [self configurationChanged];
}

- (IBAction)silenceOnErrorsOnOff:(id)sender {
    [self configurationChanged];
}
- (IBAction)onClearDebugText:(id)sender {
    NSAttributedString* attr = [[NSAttributedString alloc] initWithString:@""];
    [[_debugTextView textStorage] setAttributedString:attr];
}

-(void) configurationChanged {
    Event e;
    e.uiEvent = UIEvent::EditorConfigured;
    e.Configuration.silenceOnErrors = self.silenceOnErrors;
    e.Configuration.debug = self.debug;
    bus->publishAsync(e);
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
