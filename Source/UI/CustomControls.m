//
//  CustomControls.m
//  MidiManager
//
//  Created by Hossam Karim on 9/17/14.
//
//

#import <Cocoa/Cocoa.h>
#import "CustomControls.h"

@implementation EditorView

/*
- (void)drawRect:(NSRect)dirtyRect {
    // set any NSColor for filling, say white:
    [[NSColor blueColor] setFill];
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

@synthesize controlCode;
@synthesize label;
@synthesize slider;
@synthesize indicator;
@synthesize width;
@synthesize name;

@synthesize sliderMin;
@synthesize sliderMax;
@synthesize sliderCurrent;

@synthesize sliderActionTarget;
@synthesize sliderAction;

-(void) dealloc {
    NSLog(@"dealloc %@", self);
    NSLog(@"%@",[NSThread callStackSymbols]);
    [name release];
    [label release];
    [slider release];
    [indicator release];
    [super dealloc];
}

-(int) currentIntValue {
    return slider.intValue;
}

-(void) setCurrentIntValue:(int)currentIntValue {
    slider.intValue = currentIntValue;
}

-(NSString*) currentStringValue {
    return slider.stringValue;
}

-(void) setCurrentStringValue:(NSString *)currentStringValue {
    slider.stringValue = currentStringValue;
}

-(void) createControls {

    [self setFrameSize:NSMakeSize(width, 50)];
        
    label = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, width / 4, 17)];
    [label setStringValue:name];
    [label setBezeled:NO];
    [label setDrawsBackground:NO];
    [label setEditable:NO];
    [label setSelectable:NO];
    [[label cell] setControlSize:NSMiniControlSize];
    [[label cell] setAlignment:NSRightTextAlignment];
    [self addSubview:label];
    
    slider = [[NSSlider alloc] initWithFrame:NSMakeRect((width / 4) + 10, 0, width / 2, 17)];
    [slider setAllowsTickMarkValuesOnly:YES];
    [slider setMinValue:sliderMin];
    [slider setMaxValue:sliderMax];
    [slider setNumberOfTickMarks:sliderMax];
    [slider setAllowsTickMarkValuesOnly:YES];
    [slider setTickMarkPosition:NSTickMarkAbove];
    [[slider cell] setControlSize:NSMiniControlSize];
    [slider setIntValue:sliderCurrent];
    [slider setContinuous:NO];
    [slider setTarget:self];
    [slider setAction:@selector(onSliderChanged:)];
    [self addSubview:slider];
    
    
    indicator = [[NSTextField alloc] initWithFrame:NSMakeRect(width - (width / 4) + 20, 0, width / 4, 17)];
    [indicator setIntValue:sliderCurrent];
    [indicator setBezeled:NO];
    [indicator setDrawsBackground:NO];
    [indicator setEditable:NO];
    [indicator setSelectable:NO];
    [[indicator cell] setControlSize:NSMiniControlSize];
    [[indicator cell] setAlignment:NSLeftTextAlignment];
    [self addSubview:indicator];
    
}

-(IBAction) onSliderChanged: (NSSlider*) sender {
    NSLog(@"onSliderChanged %@, code=%d", self, controlCode);
    self.sliderCurrent = sender.intValue;
    self.indicator.intValue = sender.intValue;
    
    if ([sliderActionTarget respondsToSelector:sliderAction]) {
        [sliderActionTarget performSelector:sliderAction withObject:self];
    }
}

-(NSView*) view {
    return self;
}



@end
