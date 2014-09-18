//
//  CustomControls.h
//  MidiManager
//
//  Created by Hossam Karim on 9/17/14.
//
//

@protocol CustomControl <NSObject>

@property int controlCode;
@property (copy) NSString* name;
@property int currentIntValue;
@property (copy) NSString* currentStringValue;
@property (readonly) NSView* view;

//-(int) controlCode;
//-(NSString*) name;
//-(int) currentIntValue;
//-(NSString*) currentStringValue;
//-(NSView*) view;

@end

@interface EditorView : NSView
@end

@interface EditorScrollView : NSScrollView

@end



@interface SliderView : NSView <CustomControl> {
    int controlCode;
    NSTextField* label;
    NSSlider* slider;
    NSTextField* indicator;
    CGFloat width;
    NSString* name;
    
    int sliderMin;
    int sliderMax;
    int sliderCurrent;
    
    id sliderActionTarget;
    SEL sliderAction;
    
}

@property int controlCode;
@property (readonly) NSTextField* label;
@property (readonly) NSSlider* slider;
@property (readonly) NSTextField* indicator;
@property CGFloat width;
@property (copy) NSString* name;

@property int sliderMin;
@property int sliderMax;
@property int sliderCurrent;

@property (assign) id sliderActionTarget;
@property SEL sliderAction;


-(void) createControls;

@end