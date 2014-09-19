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

-(void) createControls;

@end

@interface EditorView : NSView
@end

@interface EditorScrollView : NSScrollView
@end



@interface SliderView : NSView <CustomControl>

@property int controlCode;
@property (readonly) NSTextField* label;
@property (readonly) NSSlider* slider;
@property (readonly) NSTextField* indicator;
@property CGFloat width;
@property (copy) NSString* name;

@property int sliderMin;
@property int sliderMax;
@property int controlCurrentValue;

@property (assign) id controlActionTarget;
@property SEL controlAction;


-(void) createControls;

@end

@interface SegmentedView : NSView <CustomControl>

@property int controlCode;
@property (readonly) NSTextField* label;
@property (readonly) NSSegmentedControl* segmentedControl;
@property CGFloat width;
@property (copy) NSString* name;
@property (copy) NSArray* buttonLabels;
@property int controlCurrentValue;

@property (assign) id controlActionTarget;
@property SEL controlAction;

@end



@interface PopUpView : NSView <CustomControl>
@property int controlCode;
@property (readonly) NSTextField* label;
@property (readonly) NSPopUpButton* popupControl;
@property CGFloat width;
@property (copy) NSString* name;
@property (copy) NSArray* menuLabels;
@property int controlCurrentValue;

@property (assign) id controlActionTarget;
@property SEL controlAction;

@end

@interface CheckBoxView : NSView <CustomControl>
@property int controlCode;
@property (readonly) NSTextField* label;
@property (readonly) NSButton* buttonControl;
@property CGFloat width;
@property (copy) NSString* name;
@property int controlCurrentValue;

@property (assign) id controlActionTarget;
@property SEL controlAction;

@end





