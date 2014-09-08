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

-(void) dealloc {
    if (listener)
        delete listener;
    [super dealloc];
}

-(void) setMessageBus: (MessageBus*) encapsulatedMessageBus {
    bus = encapsulatedMessageBus;
    listener = new NativeEditorControllerListener(self, bus);
}


-(Event) initEvent {
    Event event {};
    NSRect rect = [[self view] frame];
    event.ViewCoordinates.w = rect.size.width;
    event.ViewCoordinates.h = rect.size.height;
    return event;
}

-(void) editorLoaded {
    

    self.loadedFileName = @"Nothing Loaded right now";
    self.errors = @"No errors";
    
    Event e = [self initEvent];
    
    e.uiEvent = UIEvent::EditorLoaded;
    
    bus->publish(e);
}

-(void) configurationChanged {
    Event e;
    e.uiEvent = UIEvent::EditorConfigured;
    e.Configuration.silenceOnErrors = self.silenceOnErrors;
    e.Configuration.debug = self.debug;
    bus->publishAsync(e);
}


-(void) onEvent:(const Event&) event {
    switch (event.uiEvent) {
            
        case UIEvent::ScriptErrors:
            
            [self onScriptErrors: event];
            break;
            
        case UIEvent::EditorRestoreState:

            [self onRestoreState: event];
            break;
            
        case UIEvent::Logging:
            
            [self onLogging: event];
            break;
            
        default:
            break;
    }
    
}

-(void) onScriptErrors: (const Event&) event {
    [self setErrors:[NSString stringWithFormat:@"%s", event.ScriptData.compilationErrors.c_str()]];
}

-(void) onRestoreState: (const Event&) event {
    [self setLoadedFileName:[NSString stringWithFormat:@"%s", event.FileData.filename.c_str()]];
    if (!event.ScriptData.compilationErrors.empty()) {
        [self setErrors:[NSString stringWithFormat:@"%s", event.ScriptData.compilationErrors.c_str()]];
    } else {
        [self setErrors:@"Compiled Successfully"];
    }
}

-(void) onLogging: (const Event&) event {
    
    NSString* in = [NSString stringWithFormat:@"%s", event.MidiStream.input.c_str()];
    NSString* out = [NSString stringWithFormat:@"%s", event.MidiStream.output.c_str()];
    [self log:in withOutput:out];

}

-(void) log:(NSString*) input withOutput:(NSString*) output {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSFont* font = [NSFont fontWithName:@"Menlo" size:12];
        NSColor* inColor = [NSColor colorWithCalibratedRed:(255/255.0f) green:(189/255.0f) blue:(100/255.0f) alpha:1];
        NSColor* outColor = [NSColor colorWithCalibratedRed:(120/255.0f) green:(158/255.0f) blue:(125/255.0f) alpha:1];
        NSDictionary* inAttributes =
        @{
          NSFontAttributeName :font,
          NSForegroundColorAttributeName :inColor
        };
        NSDictionary* outAttributes =
        @{
          NSFontAttributeName :font,
          NSForegroundColorAttributeName :outColor
        };

        NSAttributedString* inattr = [[NSAttributedString alloc] initWithString:input attributes:inAttributes];
        NSAttributedString* outattr = [[NSAttributedString alloc] initWithString:output attributes:outAttributes];
            
        [[_debugTextView textStorage] appendAttributedString:inattr];
        [[_debugTextView textStorage] appendAttributedString:outattr];
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
