#!/usr/bin/python
#
# KiMony configuration file
#
from remote import *

SCREEN_WIDTH	= 240
SCREEN_HEIGHT	= 320
BUTTON_COLUMNS	= 4
BUTTON_ROWS	    = 6

BUTTON_WIDTH	= SCREEN_WIDTH/BUTTON_COLUMNS
BUTTON_HEIGHT	= SCREEN_HEIGHT/BUTTON_ROWS

# Actions
power_action = IrAction([IrCode(IrEncoding_RC6, 21, 0xFFB38), IrCode(IrEncoding_RC6, 21, 0xEFB38), 
			             IrCode(IrEncoding_NOP, 0, 250), IrCode(IrEncoding_SIRC, 12, 0xA90)])

numeric1_action = IrAction([IrCode(IrEncoding_SIRC, 12, 0x010)])
numeric2_action = IrAction([IrCode(IrEncoding_SIRC, 12, 0x810)])
numeric3_action = IrAction([IrCode(IrEncoding_SIRC, 12, 0x410)])
numeric4_action = IrAction([IrCode(IrEncoding_SIRC, 12, 0xC10)])
numeric5_action = IrAction([IrCode(IrEncoding_SIRC, 12, 0x210)])
numeric6_action = IrAction([IrCode(IrEncoding_SIRC, 12, 0xA10)])
numeric7_action = IrAction([IrCode(IrEncoding_SIRC, 12, 0x610)])
numeric8_action = IrAction([IrCode(IrEncoding_SIRC, 12, 0xE10)])
numeric9_action = IrAction([IrCode(IrEncoding_SIRC, 12, 0x110)])
numeric0_action = IrAction([IrCode(IrEncoding_SIRC, 12, 0x910)])

volume_up_action 	= IrAction([IrCode(IrEncoding_RC6, 21, 0xEEFEF, 0x10000)])
volume_down_action 	= IrAction([IrCode(IrEncoding_RC6, 21, 0xEEFEE, 0x10000)])
mute_action 		= IrAction([IrCode(IrEncoding_RC6, 21, 0xEEFF2, 0x10000)])
surround_action		= IrAction([IrCode(IrEncoding_RC6, 21, 0xEEFAD, 0x10000)])

channel_up_action 	= IrAction([IrCode(IrEncoding_SIRC, 12, 0x090)])
channel_down_action	= IrAction([IrCode(IrEncoding_SIRC, 12, 0x890)])
info_action		    = IrAction([IrCode(IrEncoding_SIRC, 12, 0x5D0)])

red_action 		    = IrAction([IrCode(IrEncoding_SIRC, 15, 0x52E9)])
yellow_action 		= IrAction([IrCode(IrEncoding_SIRC, 15, 0x72E9)])
green_action 		= IrAction([IrCode(IrEncoding_SIRC, 15, 0x32E9)])
blue_action	 	    = IrAction([IrCode(IrEncoding_SIRC, 15, 0x12E9)])

guide_action 		= IrAction([IrCode(IrEncoding_SIRC, 15, 0x6D25)])
enter_action 		= IrAction([IrCode(IrEncoding_SIRC, 12, 0xA70)])
back_action 		= IrAction([IrCode(IrEncoding_SIRC, 12, 0xC70)])
home_action 		= IrAction([IrCode(IrEncoding_SIRC, 12, 0x070)])

up_action 		    = IrAction([IrCode(IrEncoding_SIRC, 12, 0x2F0)])
down_action 		= IrAction([IrCode(IrEncoding_SIRC, 12, 0xAF0)])
left_action 		= IrAction([IrCode(IrEncoding_SIRC, 12, 0x2D0)])
right_action 		= IrAction([IrCode(IrEncoding_SIRC, 12, 0xCD0)])

# Events
home_activity_event = Event(Event_HOME)
next_event 		    = Event(Event_NEXTPAGE)
prev_event 		    = Event(Event_PREVPAGE)
power_event 		= Event(Event_IRACTION, power_action)
download_event      = Event(Event_DOWNLOAD)

numeric1_event 		= Event(Event_IRACTION, numeric1_action)
numeric2_event 		= Event(Event_IRACTION, numeric2_action)
numeric3_event 		= Event(Event_IRACTION, numeric3_action)
numeric4_event 		= Event(Event_IRACTION, numeric4_action)
numeric5_event 		= Event(Event_IRACTION, numeric5_action)
numeric6_event 		= Event(Event_IRACTION, numeric6_action)
numeric7_event 		= Event(Event_IRACTION, numeric7_action)
numeric8_event 		= Event(Event_IRACTION, numeric8_action)
numeric9_event 		= Event(Event_IRACTION, numeric9_action)
numeric0_event 		= Event(Event_IRACTION, numeric0_action)

volume_up_event 	= Event(Event_IRACTION, volume_up_action)
volume_down_event 	= Event(Event_IRACTION, volume_down_action)
mute_event 		    = Event(Event_IRACTION, mute_action)
surround_event		= Event(Event_IRACTION, surround_action)

channel_up_event 	= Event(Event_IRACTION, channel_up_action)
channel_down_event	= Event(Event_IRACTION, channel_down_action)
info_event		    = Event(Event_IRACTION, info_action)

red_event		    = Event(Event_IRACTION, red_action)
yellow_event		= Event(Event_IRACTION, yellow_action)
green_event		    = Event(Event_IRACTION, green_action)
blue_event 		    = Event(Event_IRACTION, blue_action)

guide_event		    = Event(Event_IRACTION, guide_action)
enter_event		    = Event(Event_IRACTION, enter_action)
back_event		    = Event(Event_IRACTION, back_action)
home_event		    = Event(Event_IRACTION, home_action)

up_event		    = Event(Event_IRACTION, up_action)
down_event		    = Event(Event_IRACTION, down_action)
left_event		    = Event(Event_IRACTION, left_action)
right_event		    = Event(Event_IRACTION, right_action)

# Activities, button mappings and touch button pages
test_remote_activity = Activity(
    [ 
	    ButtonMapping(0x200000, info_event),
	    #ButtonMapping(0x100000, blue_event),
	    #ButtonMapping(0x080000, green_event),
	    #ButtonMapping(0x040000, yellow_event),
	    ButtonMapping(0x020000, power_event),
	    ButtonMapping(0x010000, home_activity_event),
	    ButtonMapping(0x008000, numeric1_event),
	    ButtonMapping(0x000800, numeric2_event),
	    ButtonMapping(0x000080, numeric3_event),
	    ButtonMapping(0x000008, volume_up_event),
	    ButtonMapping(0x004000, numeric4_event),
	    ButtonMapping(0x000400, numeric5_event),
	    ButtonMapping(0x000040, numeric6_event),
	    ButtonMapping(0x000004, volume_down_event),
	    ButtonMapping(0x002000, numeric7_event),
	    ButtonMapping(0x000200, numeric8_event),
	    ButtonMapping(0x000020, numeric9_event),
	    ButtonMapping(0x000002, channel_up_event),
	    ButtonMapping(0x001000, surround_event),
	    ButtonMapping(0x000100, numeric0_event),
	    ButtonMapping(0x000010, mute_event),
	    ButtonMapping(0x000001, channel_down_event),
    ],
    [
        TouchButtonPage(
        [
	        TouchButton(guide_event, "Guide",  		      0, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),
	        TouchButton(enter_event, "Enter",  BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),
	        TouchButton(back_event,  "Back", 2*BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),
	        TouchButton(home_event,  "Home", 3*BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),

	        TouchButton(red_event,    None,  	         0, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf800, True, False),
	        TouchButton(green_event,  None,   BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0x07e0, True, False),
	        TouchButton(yellow_event, None, 2*BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xffe0, True, False),
	        TouchButton(blue_event,   None, 3*BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0x001f, True, False),

	        TouchButton(up_event, "U",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2, 2*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),
	        TouchButton(down_event, "D",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2, 3*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),
	        TouchButton(left_event, "L",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2 - BUTTON_WIDTH, int(2.5*BUTTON_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),
	        TouchButton(right_event, "R",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2 + BUTTON_WIDTH, int(2.5*BUTTON_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),

#	        TouchButton(None,    None,   	         0, 2*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
#	        TouchButton(None,    None,    BUTTON_WIDTH, 2*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
#	        TouchButton(None,    None,  2*BUTTON_WIDTH, 2*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
#	        TouchButton(None,    None,  3*BUTTON_WIDTH, 2*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
#
#	        TouchButton(None,    None,   	         0, 3*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
#	        TouchButton(None,    None,    BUTTON_WIDTH, 3*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
#	        TouchButton(None,    None,  2*BUTTON_WIDTH, 3*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
#	        TouchButton(None,    None,  3*BUTTON_WIDTH, 3*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
#
#	        TouchButton(None,    None,   	         0, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
#	        TouchButton(None,    None,    BUTTON_WIDTH, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
#	        TouchButton(None,    None,  2*BUTTON_WIDTH, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
#	        TouchButton(None,    None,  3*BUTTON_WIDTH, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
#
#	        TouchButton(None,    None,   	         0, 5*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
#	        TouchButton(None,    None,    BUTTON_WIDTH, 5*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
#	        TouchButton(None,    None,  2*BUTTON_WIDTH, 5*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
#	        TouchButton(None,    None,  3*BUTTON_WIDTH, 5*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, False),
        ])
    ])
    
test_remote_event = Event(Event_ACTIVITY, test_remote_activity)

home_activity = Activity(
    [ ButtonMapping(0x010000, prev_event), ButtonMapping(0x040000, next_event), ButtonMapping(0x100000, download_event) ],
    [ 
        TouchButtonPage(
        [
            TouchButton(test_remote_event, "Test 1", 0, 0*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, False, True),   
            TouchButton(test_remote_event, "Test 2", 0, 1*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, False, True),   
        ]),
        
        TouchButtonPage(
        [
            TouchButton(test_remote_event, "Test 2", 0, 1*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, False, True),   
            TouchButton(test_remote_event, "Test 3", 0, 2*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, False, True),   
        ]),

        TouchButtonPage(
        [
            TouchButton(test_remote_event, "Test 3", 0, 2*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, False, True),   
            TouchButton(test_remote_event, "Test 4", 0, 3*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, False, True),   
        ]),
    ])

package = Package()
package.append(home_activity)
package.append(test_remote_activity)
package.append(test_remote_event)
package.append(home_activity_event)
package.append(next_event)
package.append(prev_event)
package.append(power_event)
package.append(download_event)
package.append(numeric1_event)
package.append(numeric2_event)
package.append(numeric3_event)
package.append(numeric4_event)
package.append(numeric5_event)
package.append(numeric6_event)
package.append(numeric7_event)
package.append(numeric8_event)
package.append(numeric9_event)
package.append(numeric0_event)
package.append(volume_up_event)
package.append(volume_down_event)
package.append(mute_event)
package.append(surround_event)
package.append(channel_up_event)
package.append(channel_down_event)
package.append(info_event)
package.append(red_event)
package.append(yellow_event)
package.append(green_event)
package.append(blue_event)
package.append(guide_event)
package.append(enter_event)
package.append(back_event)
package.append(home_event)
package.append(up_event)
package.append(down_event)
package.append(left_event)
package.append(right_event)

package.append(power_action)
package.append(numeric1_action)
package.append(numeric2_action)
package.append(numeric3_action)
package.append(numeric4_action)
package.append(numeric5_action)
package.append(numeric6_action)
package.append(numeric7_action)
package.append(numeric8_action)
package.append(numeric9_action)
package.append(numeric0_action)
package.append(volume_up_action)
package.append(volume_down_action)
package.append(mute_action)
package.append(surround_action)
package.append(channel_up_action)
package.append(channel_down_action)
package.append(info_action)
package.append(red_action)
package.append(yellow_action)
package.append(green_action)
package.append(blue_action)
package.append(guide_action)
package.append(enter_action)
package.append(back_action)
package.append(home_action)
package.append(up_action)
package.append(down_action)
package.append(left_action)
package.append(right_action)

packed_data = package.pack()

