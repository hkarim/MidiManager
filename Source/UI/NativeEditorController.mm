//
//  NativeEditorController.m
//  MidiManager
//
//  Created by Hossam Karim on 9/2/14.
//
//

#import "NativeEditorController.h"
#import "NS(Attributed)String+Geometrics.h"
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
        case UIWidgetType::Label:
            return [self createLabel:widget];
            break;
            
        default:
            return nil;
            break;
    }
}

-(SliderView*) createSlider: (UIWidget*) widget {
    
    UIWidgetSlider* control = static_cast<UIWidgetSlider*>(widget);
    NSString* name = [NSString stringWithFormat:@"%s", control->name.c_str()];
    
    NSRect frame = [[[self scrollView] contentView] frame];
    
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
    
    NSRect frame = [[[self scrollView] contentView] frame];
    
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
    
    NSRect frame = [[[self scrollView] contentView] frame];
    
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
    
    NSRect frame = [[[self scrollView] contentView] frame];
    
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

-(LabelView*) createLabel: (UIWidget*) widget {
    
    UIWidgetLabel* control = static_cast<UIWidgetLabel*>(widget);
    NSString* name = [NSString stringWithFormat:@"%s", control->name.c_str()];
    
    NSRect frame = [[[self scrollView] contentView] frame];
    
    LabelView* view = [[LabelView alloc] init];
    view.controlCode = control->code;
    view.name = name;
    view.width = frame.size.width;
    view.controlCurrentValue = [NSString stringWithFormat:@"%s", control->value.c_str()];
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
    //[[self view] setFrameSize:NSMakeSize(400, 300)];
    //[[self scrollView] setFrameSize:NSMakeSize(400, 300)];
    NSRect rect = [[self view] frame];
    event.ViewCoordinates.w = rect.size.width;
    event.ViewCoordinates.h = rect.size.height;
    return event;
}

-(void) editorLoaded {
    

    self.loadedFileName = @"Nothing Loaded right now";
    self.errors = [self stringForErrorsView:@"No errors"];
    
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
            
        case UIEvent::UISignal:
            
            [self onUISignal: event];
            break;
            
        default:
            break;
    }
    
}

-(NSAttributedString*) stringForErrorsView:(NSString*) input {
    NSFont* font = [NSFont fontWithName:@"Menlo" size:14];
    NSColor* inColor = [NSColor colorWithCalibratedRed:(255/255.0f) green:(189/255.0f) blue:(100/255.0f) alpha:1];
    NSDictionary* inAttributes =
    @{
      NSFontAttributeName :font,
      NSForegroundColorAttributeName :inColor
      };
    
    NSAttributedString* inattr = [[NSAttributedString alloc] initWithString:input attributes:inAttributes];
    return inattr;
}

-(NSAttributedString*) stringForErrorsViewFromStdString:(const std::string&) input {
    return [self stringForErrorsView: [NSString stringWithFormat:@"%s", input.c_str()]];
}

-(void) onScriptErrors: (const Event&) event {
    [self setErrors:[self stringForErrorsViewFromStdString: event.ScriptData.compilationErrors]];
}

-(void) onRestoreState: (const Event&) event {
    [self setLoadedFileName:[NSString stringWithFormat:@"%s", event.FileData.filename.c_str()]];
    if (!event.ScriptData.compilationErrors.empty()) {
        [self setErrors:[self stringForErrorsViewFromStdString: event.ScriptData.compilationErrors]];
    } else {
        [self setErrors:[self stringForErrorsView: @"Compiled Successfully"]];
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
    
    EditorView* editor = [[EditorView alloc] initWithFrame:NSMakeRect(x, y, w, h)];

    //NSLog(@"Setting new DocumentView %@", self);
    
    
    NSMutableArray* controls = [NSMutableArray new];
    //NSLog(@"Widgets Size: %lu", event.UI.widgets.size());
    for (auto& widget : event.UI.widgets) {
        [controls addObject:[self createWidget: widget]];
    }

    NSView* prev = nil;
    NSDictionary* views = nil;
    NSArray* breakableConstraints = nil;
    editor.translatesAutoresizingMaskIntoConstraints = NO;
    for (id control in controls) {
        NSView* controlView = [control view];
        CGFloat currentControlHeight = controlView.frame.size.height;
        NSDictionary *metrics = @{ @"w": [NSNumber numberWithDouble:w], @"ch": [NSNumber numberWithDouble:currentControlHeight] };
        controlView.translatesAutoresizingMaskIntoConstraints = NO;
        [editor addSubview:control];
        
        if (prev == nil) {
            views = NSDictionaryOfVariableBindings(control);

            // Center horizontally
            [editor addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|[control(==w)]|"
                                                                           options:NSLayoutFormatAlignAllTop
                                                                           metrics:metrics
                                                                             views:views]];

            // add to the top of the editor and constraint the height
            [editor addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[control(>=ch)]"
                                                                           options:NSLayoutFormatAlignAllLeft
                                                                           metrics:metrics
                                                                             views:views]];
            
            // add trailing constraint, and break it if another control is added
            // this indicates the control as the last one
            breakableConstraints = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[control]-|"
                                                                           options:NSLayoutFormatAlignAllLeft
                                                                           metrics:metrics
                                                                             views:views];
            [editor addConstraints:breakableConstraints];
        }
        else {
            views = NSDictionaryOfVariableBindings(control,prev);
            
            // Center horizontally
            [editor addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|[control(==w)]|"
                                                                           options:NSLayoutFormatAlignAllTop
                                                                           metrics:metrics
                                                                             views:views]];
            // break the previous vertical constraint
            [editor removeConstraints:breakableConstraints];
            
            NSString* layout = @"V:[prev]-[control(>=ch)]";
            if ([prev isKindOfClass:[LabelView class]] && [control isKindOfClass:[LabelView class]]) {
                // just remove the spacing between consecutive labels, looks nicer
                layout = @"V:[prev][control(>=ch)]"; // remove this small magic '-'
            }
            // stack under the previous control
            [editor addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:layout
                                                                           options:NSLayoutFormatAlignAllLeft
                                                                           metrics:metrics
                                                                             views:views]];
            // this is now the last one
            breakableConstraints = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[control]-|"
                                                                           options:NSLayoutFormatAlignAllLeft
                                                                           metrics:metrics
                                                                             views:views];
            [editor addConstraints:breakableConstraints];
            
        }
        
        
        prev = control;
    }
    
    [[self scrollView] setDocumentView:editor];
}

-(void) onUISignal:(Event) event {
    EditorView* editor = [_scrollView documentView];
    if (editor == nil) {
        return;
    }
    
    NSArray *views = [editor subviews];
    
    for (id view : views) {
        id<CustomControl> current = view;
        if ([current controlCode] == event.Change.code) {
            [current setCurrentIntValue:event.Change.intValue];
            [current setCurrentStringValue:[NSString stringWithFormat:@"%s", event.Change.stringValue.c_str()]];
            return;
        }
    }
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
    self.errors = [self stringForErrorsView:@"Compiling, Please wait"];
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
