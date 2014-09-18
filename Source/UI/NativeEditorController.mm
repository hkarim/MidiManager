//
//  NativeEditorController.m
//  MidiManager
//
//  Created by Hossam Karim on 9/2/14.
//
//

#import "NativeEditorController.h"
#include <sstream>

@interface NativeEditorController ()
@property (assign) IBOutlet NSTextView *debugTextView;
@property (assign) IBOutlet NSButton *reloadScriptButton;
@property (assign) IBOutlet EditorScrollView *scrollView;
@end

@implementation NativeEditorController

__weak MessageBus* bus;



@synthesize loadedFileName;
@synthesize errors;
@synthesize debug;
@synthesize silenceOnErrors;

-(void) dealloc {
    NSLog(@"dealloc %@", self);
    
    if (listener)
        delete listener;
    
    [self removeCustomControls];
    
    [super dealloc];
}

-(void) removeCustomControls {
    NSLog(@"removeCustomControls %@", self);
    
    /*
    for (auto& v : controls) {
        id<CustomControl> control = v.second;
        NSView* view = [control view];
        [view removeFromSuperview];
        [control release];
    }
    
    controls.clear();
    */
}

-(SliderView*) createSlider: (UIWidget*) widget {
    
    UIWidgetSlider* slider = static_cast<UIWidgetSlider*>(widget);
    NSString* name = [NSString stringWithFormat:@"%s", slider->name.c_str()];
    NSLog(@"createSlider: %@ code:%d value:%d min:%d max:%d", name, slider->code, slider->value, slider->minimum, slider->maximum);
    SliderView* customControl =
    [self createSliderNamed:name code: slider->code value:slider->value minimum:slider->minimum maximum:slider->maximum];
    //int code = slider->code;
    //controls.insert(std::map<int, id<CustomControl>>::value_type(code, customControl));
    NSLog(@"created %@ by %@", customControl, self);
    return customControl;
}

-(SliderView*) createSliderNamed:(NSString*) name code:(int) code value:(int) value minimum:(int) min maximum:(int) max {
    NSRect frame = [[[self scrollView] documentView] frame];
    
    SliderView* view = [[SliderView alloc] init];
    view.controlCode = code;
    view.name = name;
    view.width = frame.size.width;
    view.sliderCurrent = value;
    view.sliderMin = min;
    view.sliderMax = max;
    view.sliderActionTarget = self;
    view.sliderAction = @selector(customControlAction:);
    [view createControls];
    
    return view;
    
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
        if (!event.FileData.filename.empty() && !event.UI.widgets.empty()) {
            [self onCreateUI:event];
        }
    }
    [[self reloadScriptButton] setEnabled:YES];
}

-(void) onLogging: (const Event&) event {
    
    NSString* in = [NSString stringWithFormat:@"%s", event.Logging.input.c_str()];
    NSString* out = [NSString stringWithFormat:@"%s", event.Logging.output.c_str()];
    [self log:in withOutput:out];

}

-(void) onCreateUI: (const Event&) event {
    NSLog(@"onCreateUI %@", self);
    //[self removeCustomControls];
    
    
    NSRect frame = [[self scrollView] frame];
    CGFloat x = frame.origin.x;
    CGFloat y = frame.origin.y;
    CGFloat w = frame.size.width;
    CGFloat h = frame.size.height;
    
    EditorView* editor =
      [[EditorView alloc] initWithFrame:NSMakeRect(x, y, w, h)];
    
    NSLog(@"Setting new DocumentView %@", self);
    [[self scrollView] setDocumentView:editor];
    
    NSMutableArray* controls = [NSMutableArray new];
    
    for (auto& widget : event.UI.widgets) {
        switch (widget->widgetType) {
            case UIWidgetType::Slider:
                [controls addObject:[self createSlider: widget]];
                break;
                
            default:
                break;
        }
    }

    int yinc = 0;
    for (id control in controls) {
        [control setFrameOrigin:NSMakePoint(x, y + yinc)];
        [editor addSubview:control];
        yinc += 20;
    }
    [controls release];
    
    [editor setFrameSize:NSMakeSize(w-1, y + yinc + 100)];
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

- (IBAction) customControlAction : (id) sender {
    NSLog(@"customControlAction %@ sent object %@", self, sender);
    id control = (id<CustomControl>) sender;
    int code = [control controlCode];  // DO NOT DELETE THIS USELESS LINE

    Event e = [self initEvent];
    e.uiEvent = UIEvent::Logging;
    std::ostringstream input, output;
    
    NSString* name = [control name];
    input << "Control {code-" << [control controlCode] << "} " << [name UTF8String] << std::endl;
    output << "New Value: " << [control currentIntValue] << std::endl << std::endl;
    e.Logging.input = input.str();
    e.Logging.output = output.str();
    bus->publishAsync(e);
    
    
    int intValue = [control currentIntValue];
    std::string stringValue { [[control currentStringValue] UTF8String] };

    e = [self initEvent];
    e.uiEvent = UIEvent::EditorWidgetChanged;
    e.Change.code = [control controlCode];
    e.Change.intValue = intValue;
    e.Change.stringValue = stringValue;
    bus->publishAsync(e);
    
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
        [[self reloadScriptButton] setEnabled:YES];
    }
    
}

- (IBAction)reloadScriptClicked:(NSButton *)sender {
    if (self.loadedFileName) {
        Event e = [self initEvent];
        e.uiEvent = UIEvent::FileChosen;
        e.FileData.filename = [self.loadedFileName UTF8String];
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
