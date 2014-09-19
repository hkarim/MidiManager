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
    //NSLog(@"dealloc %@", self);
    
    if (listener)
        delete listener;

    
    [super dealloc];
}

-(id<CustomControl>) createWidget:(UIWidget*) widget {
    switch (widget->widgetType) {
            
        case UIWidgetType::Slider:
            return [self createSlider:widget];
            break;
            
        case UIWidgetType::Segmented:
            return [self createSegmented:widget];
            break;
            
        case UIWidgetType::PopUp:
            return [self createPopUp:widget];
            break;
        case UIWidgetType::CheckBox:
            return [self createCheckBox:widget];
            break;
            
        default:
            return nil;
            break;
    }
}

-(SliderView*) createSlider: (UIWidget*) widget {
    
    UIWidgetSlider* control = static_cast<UIWidgetSlider*>(widget);
    NSString* name = [NSString stringWithFormat:@"%s", control->name.c_str()];
    
    NSRect frame = [[[self scrollView] documentView] frame];
    
    SliderView* view = [[SliderView alloc] init];
    view.controlCode = control->code;
    view.name = name;
    view.width = frame.size.width;
    view.controlCurrentValue = control->value;
    view.sliderMin = control->minimum;
    view.sliderMax = control->maximum;
    view.controlActionTarget = self;
    view.controlAction = @selector(customControlAction:);
    [view createControls];
    
    
    return view;
}

-(SegmentedView*) createSegmented: (UIWidget*) widget {
    
    UIWidgetSegmented* control = static_cast<UIWidgetSegmented*>(widget);
    NSString* name = [NSString stringWithFormat:@"%s", control->name.c_str()];
    
    NSRect frame = [[[self scrollView] documentView] frame];
    
    NSMutableArray* labels = [NSMutableArray new];
    
    for (auto& v : control->labels) {
        [labels addObject:[NSString stringWithFormat:@"%s", v.c_str()]];
    }
    
    SegmentedView* view = [[SegmentedView alloc] init];
    view.controlCode = control->code;
    view.name = name;
    view.width = frame.size.width;
    view.buttonLabels = labels;
    view.controlCurrentValue = control->value;
    view.controlActionTarget = self;
    view.controlAction = @selector(customControlAction:);
    [view createControls];
    
    
    return view;
}

-(PopUpView*) createPopUp: (UIWidget*) widget {
    
    UIWidgetPopUp* control = static_cast<UIWidgetPopUp*>(widget);
    NSString* name = [NSString stringWithFormat:@"%s", control->name.c_str()];
    
    NSRect frame = [[[self scrollView] documentView] frame];
    
    NSMutableArray* labels = [NSMutableArray new];
    
    for (auto& v : control->labels) {
        [labels addObject:[NSString stringWithFormat:@"%s", v.c_str()]];
    }
    
    PopUpView* view = [[PopUpView alloc] init];
    view.controlCode = control->code;
    view.name = name;
    view.width = frame.size.width;
    view.menuLabels = labels;
    view.controlCurrentValue = control->value;
    view.controlActionTarget = self;
    view.controlAction = @selector(customControlAction:);
    [view createControls];
    
    
    return view;
}

-(CheckBoxView*) createCheckBox: (UIWidget*) widget {
    
    UIWidgetCheckBox* control = static_cast<UIWidgetCheckBox*>(widget);
    NSString* name = [NSString stringWithFormat:@"%s", control->name.c_str()];
    
    NSRect frame = [[[self scrollView] documentView] frame];
    
    CheckBoxView* view = [[CheckBoxView alloc] init];
    view.controlCode = control->code;
    view.name = name;
    view.width = frame.size.width;
    view.controlCurrentValue = control->value;
    view.controlActionTarget = self;
    view.controlAction = @selector(customControlAction:);
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
    std::lock_guard<std::mutex> lock(mutex);
    //NSLog(@"onCreateUI %@", self);
    //[self removeCustomControls];
    
    
    NSRect frame = [[self scrollView] frame];
    CGFloat x = frame.origin.x;
    CGFloat y = frame.origin.y;
    CGFloat w = frame.size.width;
    CGFloat h = frame.size.height;
    
    NSView* prevEditor = (EditorView*) [[self scrollView] documentView];
    if (prevEditor) {
        [[self scrollView] setDocumentView:nil];
        NSArray* views = [prevEditor subviews];
        for (id v in views) {
            //NSLog(@"EditorView subview %@", v);
            [v release];
        }
    }
    
    EditorView* editor =
      [[EditorView alloc] initWithFrame:NSMakeRect(x, y, w, h)];
    
    //NSLog(@"Setting new DocumentView %@", self);
    [[self scrollView] setDocumentView:editor];
    
    NSMutableArray* controls = [NSMutableArray new];
    //NSLog(@"Widgets Size: %lu", event.UI.widgets.size());
    for (auto& widget : event.UI.widgets) {
        [controls addObject:[self createWidget: widget]];
    }

    CGFloat yinc = 0;
    for (id control in controls) {
        NSView* controlView = [control view];
        
        [control setFrameOrigin:NSMakePoint(x, yinc)];
        [editor addSubview:control];
        
        yinc += controlView.frame.size.height + 7;
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
    std::lock_guard<std::mutex> lock(mutex);
    
    //NSLog(@"customControlAction %@ sent object %@", self, sender);
    id control = (id<CustomControl>) sender;

    /*
    Event e = [self initEvent];
    e.uiEvent = UIEvent::Logging;
    std::ostringstream input, output;
    
    NSString* name = [control name];
    input << "Control {code-" << [control controlCode] << "} " << [name UTF8String] << std::endl;
    output << "New Value: " << [control currentIntValue] << std::endl << std::endl;
    e.Logging.input = input.str();
    e.Logging.output = output.str();
    bus->publishAsync(e);
    */
    
    int intValue = [control currentIntValue];
    std::string stringValue { [[control currentStringValue] UTF8String] };
    Event e = [self initEvent];
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
