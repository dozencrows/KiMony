

from remote import *

power_action = IrAction([IrCode(IrEncoding_RC6, 21, 0xFFB38), IrCode(IrEncoding_RC6, 21, 0xEFB38), IrCode(IrEncoding_NOP, 0, 250), IrCode(IrEncoding_SIRC, 12, 0xA90)])

home_event = Event(Event_HOME, None)
next_event = Event(Event_NEXTPAGE, None)
prev_event = Event(Event_PREVPAGE, None)
power_event = Event(Event_IRACTION, power_action)

test_remote_activity = Activity(
    [ 
        ButtonMapping(0x010000, home_event), 
    ],
    [ 
        TouchButtonPage(
        [
            TouchButton(),   
        ]),
        
        TouchButtonPage(
        [
            TouchButton(),   
        ]),
    ])
    
test_remote_event = Event(Event_ACTIVITY, test_remote_activity)

home_activity = Activity(
    [ ButtonMapping(0x010000, prev_event), ButtonMapping(0x040000, next_event) ],
    [ 
        TouchButtonPage(
        [
            TouchButton(),   
        ]),
        
        TouchButtonPage(
        [
            TouchButton(),   
        ]),
    ])
