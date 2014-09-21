//
//  CustomControls.m
//  MidiManager
//
//  Created by Hossam Karim on 9/17/14.
//
//

#import <Cocoa/Cocoa.h>
#import "CustomControls.h"
#import "NS(Attributed)String+Geometrics.h"

@implementation EditorView

/*
- (void)drawRect:(NSRect)dirtyRect {
    // set any NSColor for filling, say white:
    [[NSColor yellowColor] setFill];
    NSRectFill(dirtyRect);
    [super drawRect:dirtyRect];
}
*/


-(BOOL) isFlipped {
    return YES;
}

@end

@implementation EditorScrollView

-(BOOL) isFlipped {
    return YES;
}

@end


@implementation SliderView

-(int) currentIntValue {
    return _slider.intValue;
}

-(void) setCurrentIntValue:(int)currentIntValue {
    _slider.intValue = currentIntValue;
}

-(NSString*) currentStringValue {
    return _slider.stringValue;
}

-(void) setCurrentStringValue:(NSString *)currentStringValue {
    _slider.stringValue = currentStringValue;
}

-(void) createControls {

    NSControlSize defaultControlSize = NSSmallControlSize;
    CGFloat defaultSize = [NSFont systemFontSizeForControlSize:defaultControlSize];
    NSFont* font = [NSFont systemFontOfSize:defaultSize];
    
    [self setFrameSize:NSMakeSize(_width, defaultSize * 2)];
    

    _label = [[NSTextField alloc] init];
    _label.translatesAutoresizingMaskIntoConstraints = NO;
    [_label setStringValue:_name];
    [_label setBezeled:NO];
    [_label setDrawsBackground:NO];
    [_label setEditable:NO];
    [_label setSelectable:NO];
    [[_label cell] setControlSize:defaultControlSize];
    [_label setFont:font];
    [[_label cell] setAlignment:NSRightTextAlignment];
    [self addSubview:_label];
    

    _slider = [[NSSlider alloc] init];
    _slider.translatesAutoresizingMaskIntoConstraints = NO;
    [_slider setAllowsTickMarkValuesOnly:YES];
    [_slider setMinValue:_sliderMin];
    [_slider setMaxValue:_sliderMax];
    [_slider setNumberOfTickMarks:_sliderMax];
    [_slider setAllowsTickMarkValuesOnly:YES];
    [_slider setTickMarkPosition:NSTickMarkAbove];
    [[_slider cell] setControlSize:defaultControlSize];
    [_slider setContinuous:NO];
    [_slider setTarget:self];
    [_slider setIntValue:_controlCurrentValue];
    [_slider setAction:@selector(onControlChanged:)];
    [self addSubview:_slider];
    
    
    _indicator = [[NSTextField alloc] init];
    _indicator.translatesAutoresizingMaskIntoConstraints = NO;
    [_indicator setIntValue:_controlCurrentValue];
    [_indicator setBezeled:NO];
    [_indicator setDrawsBackground:NO];
    [_indicator setEditable:NO];
    [_indicator setSelectable:NO];
    [[_indicator cell] setControlSize:defaultControlSize];
    [_indicator setFont:font];
    [[_indicator cell] setAlignment:NSLeftTextAlignment];
    [self addSubview:_indicator];
    
    // Layout
    NSDictionary *metrics = @{ @"wp": @100.0 };
    NSDictionary *views = NSDictionaryOfVariableBindings(_label,_slider,_indicator);
    
    // Horizontal layout - note the options for aligning the top and bottom of all views
    [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|-[_label(wp)]-[_slider]-[_indicator(wp)]-|"
                                                                 options:NSLayoutFormatAlignAllBottom
                                                                 metrics:metrics
                                                                   views:views]];
    
}

-(IBAction) onControlChanged: (NSSlider*) sender {
    //NSLog(@"onSliderChanged %@, code=%d", self, controlCode);
    _controlCurrentValue = sender.intValue;
    _indicator.intValue = sender.intValue;
    
    if ([_controlActionTarget respondsToSelector:_controlAction]) {
        [_controlActionTarget performSelector:_controlAction withObject:self];
    }
}

-(NSView*) view {
    return self;
}

@end





@implementation SegmentedView

-(int) currentIntValue {
    return _segmentedControl.selectedSegment;
}

-(void) setCurrentIntValue:(int)currentIntValue {
    _segmentedControl.selectedSegment = currentIntValue;
}

-(NSString*) currentStringValue {
    return @"";
}

-(void) setCurrentStringValue:(NSString *)currentStringValue {

}


-(void) createControls {
    
    NSControlSize defaultControlSize = NSSmallControlSize;
    CGFloat defaultSize = [NSFont systemFontSizeForControlSize:defaultControlSize];
    NSFont* font = [NSFont systemFontOfSize:defaultSize];
    
    [self setFrameSize:NSMakeSize(_width, defaultSize * 2.3)];
    
    _label = [[NSTextField alloc] init];
    _label.translatesAutoresizingMaskIntoConstraints = NO;
    [_label setStringValue:_name];
    [_label setBezeled:NO];
    [_label setDrawsBackground:NO];
    [_label setEditable:NO];
    [_label setSelectable:NO];
    [[_label cell] setControlSize:defaultControlSize];
    [_label setFont:font];
    [[_label cell] setAlignment:NSRightTextAlignment];
    [self addSubview:_label];
    
    
    _segmentedControl = [[NSSegmentedControl alloc] init];
    _segmentedControl.translatesAutoresizingMaskIntoConstraints = NO;
    [_segmentedControl setSegmentStyle:NSSegmentStyleRoundRect];
    [_segmentedControl setAutoresizesSubviews:YES];
    [_segmentedControl setContinuous:NO];
    [_segmentedControl setTarget:self];
    [[_segmentedControl cell] setControlSize:defaultControlSize];
    [_segmentedControl setFont:font];
    [_segmentedControl setSegmentCount:[_buttonLabels count]];
    int i = 0;
    for (id label in _buttonLabels) {
        [_segmentedControl setLabel:label forSegment:i++];
    }
    _segmentedControl.selectedSegment = _controlCurrentValue;
    
    [_segmentedControl setAction:@selector(onControlChanged:)];
    
    [self addSubview:_segmentedControl];
    
    // Layout
    NSDictionary *metrics = @{ @"wp": @100.0 };
    NSDictionary *views = NSDictionaryOfVariableBindings(_label,_segmentedControl);
    
    [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|-[_label(wp)]-[_segmentedControl]-wp-|"
                                                                 options:NSLayoutFormatAlignAllBottom
                                                                 metrics:metrics
                                                                   views:views]];
    
    
}

-(NSView*) view {
    return self;
}

-(IBAction) onControlChanged:(NSSegmentedControl*)sender {
    _controlCurrentValue = [sender selectedSegment];
    if ([_controlActionTarget respondsToSelector:_controlAction]) {
        [_controlActionTarget performSelector:_controlAction withObject:self];
    }
}

@end


@implementation PopUpView

-(int) currentIntValue {
    return [_popupControl indexOfSelectedItem];
}

-(void) setCurrentIntValue:(int)currentIntValue {
    [_popupControl selectItemAtIndex:currentIntValue];
}

-(NSString*) currentStringValue {
    return [[_popupControl selectedItem] title];
}

-(void) setCurrentStringValue:(NSString *)currentStringValue {
    [_popupControl selectItemWithTitle:currentStringValue];
}


-(void) createControls {
    
    NSControlSize defaultControlSize = NSSmallControlSize;
    CGFloat defaultSize = [NSFont systemFontSizeForControlSize:defaultControlSize];
    NSFont* font = [NSFont systemFontOfSize:defaultSize];
    
    [self setFrameSize:NSMakeSize(_width, defaultSize * 2)];
    
    _label = [[NSTextField alloc] init];
    _label.translatesAutoresizingMaskIntoConstraints = NO;
    [_label setStringValue:_name];
    [_label setBezeled:NO];
    [_label setDrawsBackground:NO];
    [_label setEditable:NO];
    [_label setSelectable:NO];
    [[_label cell] setControlSize:defaultControlSize];
    [_label setFont:font];
    [[_label cell] setAlignment:NSRightTextAlignment];
    [self addSubview:_label];
    
    
    _popupControl = [[NSPopUpButton alloc] init];
    _popupControl.translatesAutoresizingMaskIntoConstraints = NO;
    [_popupControl setAutoresizesSubviews:YES];
    [_popupControl setContinuous:NO];
    [_popupControl setTarget:self];
    [[_popupControl cell] setControlSize:defaultControlSize];
    [_popupControl setFont:font];
    
    for (id label in _menuLabels) {
        [_popupControl addItemWithTitle:label];
    }
    [_popupControl selectItemAtIndex:_controlCurrentValue];
    
    [_popupControl setAction:@selector(onControlChanged:)];
    [self addSubview:_popupControl];
    
    // Layout
    NSDictionary *metrics = @{ @"wp": @100.0 };
    NSDictionary *views = NSDictionaryOfVariableBindings(_label,_popupControl);
    
    [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|-[_label(wp)]-[_popupControl]-wp-|"
                                                                 options:NSLayoutFormatAlignAllBottom
                                                                 metrics:metrics
                                                                   views:views]];
    
    
}

-(NSView*) view {
    return self;
}

-(IBAction) onControlChanged:(NSPopUpButton*)sender {
    _controlCurrentValue = [_popupControl indexOfSelectedItem];
    if ([_controlActionTarget respondsToSelector:_controlAction]) {
        [_controlActionTarget performSelector:_controlAction withObject:self];
    }
}

@end




@implementation CheckBoxView

-(int) currentIntValue {
    return [_buttonControl state];
}

-(void) setCurrentIntValue:(int)currentIntValue {
    if (currentIntValue == 0) {
        [_buttonControl setState:NSOffState];
    } else if (currentIntValue == 1) {
        [_buttonControl setState:NSOnState];
    }
    
}

-(NSString*) currentStringValue {
    return @"";
}

-(void) setCurrentStringValue:(NSString *)currentStringValue {
    
}


-(void) createControls {
    
    NSControlSize defaultControlSize = NSSmallControlSize;
    CGFloat defaultSize = [NSFont systemFontSizeForControlSize:defaultControlSize];
    NSFont* font = [NSFont systemFontOfSize:defaultSize];
    
    [self setFrameSize:NSMakeSize(_width, defaultSize * 2)];
    
    _label = [[NSTextField alloc] init];
    _label.translatesAutoresizingMaskIntoConstraints = NO;
    [_label setStringValue:_name];
    [_label setBezeled:NO];
    [_label setDrawsBackground:NO];
    [_label setEditable:NO];
    [_label setSelectable:NO];
    [[_label cell] setControlSize:defaultControlSize];
    [_label setFont:font];
    [[_label cell] setAlignment:NSRightTextAlignment];
    [self addSubview:_label];
    
    
    _buttonControl = [[NSButton alloc] init];
    _buttonControl.translatesAutoresizingMaskIntoConstraints = NO;
    [_buttonControl setButtonType:NSSwitchButton];
    [_buttonControl setTitle:@""];
    [_buttonControl setAutoresizesSubviews:YES];
    [_buttonControl setContinuous:NO];
    [_buttonControl setTarget:self];
    [[_buttonControl cell] setControlSize:defaultControlSize];
    [_buttonControl setFont:font];
    [self setCurrentIntValue:_controlCurrentValue];
    [_buttonControl setAction:@selector(onControlChanged:)];
    [self addSubview:_buttonControl];
    
    // Layout
    NSDictionary *metrics = @{ @"wp": @100.0 };
    NSDictionary *views = NSDictionaryOfVariableBindings(_label,_buttonControl);
    
    [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|-[_label(wp)]-[_buttonControl]-wp-|"
                                                                 options:NSLayoutFormatAlignAllBottom
                                                                 metrics:metrics
                                                                   views:views]];
    
    
}

-(NSView*) view {
    return self;
}

-(IBAction) onControlChanged:(NSPopUpButton*)sender {
    _controlCurrentValue = [_buttonControl state];
    if ([_controlActionTarget respondsToSelector:_controlAction]) {
        [_controlActionTarget performSelector:_controlAction withObject:self];
    }
}

@end


@implementation LabelView

-(int) currentIntValue {
    return 0;
}

-(void) setCurrentIntValue:(int)currentIntValue {
    
}

-(NSString*) currentStringValue {
    return [_contentControl stringValue];
}

-(void) setCurrentStringValue:(NSString *)currentStringValue {
    _contentControl.stringValue = currentStringValue;
}


- (void)drawRect:(NSRect)dirtyRect {
    [[NSColor lightGrayColor] setFill];
    NSRectFill(dirtyRect);
    [super drawRect:dirtyRect];
}
 

-(void) createControls {
    
    NSControlSize defaultControlSize = NSRegularControlSize;
    CGFloat defaultSize = [NSFont systemFontSizeForControlSize:defaultControlSize];
    NSFont* font = [NSFont systemFontOfSize:defaultSize];
    NSFont* boldFont = [NSFont boldSystemFontOfSize:defaultSize];

    CGFloat length = _controlCurrentValue.length;
    CGFloat height = defaultSize * 1.61803398875f * (length / 100.0f) * 3.0f;
    if (height < 21.0f) height = defaultSize * 1.61803398875f * 3.0f;
    
    //NSLog(@"H: %f", height);
    [self setFrameSize:NSMakeSize(_width, height)];
    
    _label = [[NSTextField alloc] init];
    _label.translatesAutoresizingMaskIntoConstraints = NO;
    [_label setStringValue:_name];
    [_label setBezeled:NO];
    [_label setDrawsBackground:NO];
    [_label setEditable:NO];
    [_label setSelectable:YES];
    [_label setFont:boldFont];
    [[_label cell] setAlignment:NSLeftTextAlignment];
    [self addSubview:_label];
    
    
    _contentControl = [[NSTextField alloc] init];
    _contentControl.translatesAutoresizingMaskIntoConstraints = NO;
    [_contentControl setStringValue:_controlCurrentValue];
    //[[_contentControl cell] setLineBreakMode:NSLineBreakByWordWrapping];
    //[[_contentControl cell] setUsesSingleLineMode:NO];
    [_contentControl setBezeled:NO];
    [_contentControl setEditable:NO];
    [_contentControl setSelectable:YES];
    [_contentControl setDrawsBackground:NO];
    //[_contentControl setDrawsBackground:YES];
    //[_contentControl setBackgroundColor:[NSColor blueColor]];
    [_contentControl setFont:font];
    [[_contentControl cell] setAlignment:NSLeftTextAlignment];
    [self addSubview:_contentControl];
    
    // Layout

    NSDictionary *metrics = @{ @"wp": @100.0, @"vp": [NSNumber numberWithDouble:height] };
    NSDictionary *views = NSDictionaryOfVariableBindings(_label,_contentControl);
    
    [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|-[_label(wp)]-[_contentControl]-|"
                                                                 options:NSLayoutFormatAlignAllTop
                                                                 metrics:metrics
                                                                   views:views]];
    [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|-[_contentControl(<=vp)]-|"
                                                                 options:nil
                                                                 metrics:metrics
                                                                   views:views]];
    [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|-[_label(_contentControl)]-|"
                                                                 options:nil
                                                                 metrics:metrics
                                                                   views:views]];
    
}

-(NSView*) view {
    return self;
}

-(IBAction) onControlChanged:(NSPopUpButton*)sender {
    _controlCurrentValue = [_contentControl stringValue];
    if ([_controlActionTarget respondsToSelector:_controlAction]) {
        [_controlActionTarget performSelector:_controlAction withObject:self];
    }
}

@end










