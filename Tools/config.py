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

sony_tv = Device(
                 options = [ 
                            Option(name = "power", flags = Option_Cycled|Option_DefaultToZero|Option_ActionOnDefault, max_value = 1, change_action_names = ["powertoggle"]),
                            Option(name = "input", flags = Option_Cycled, max_value = 5, change_action_names = ["nextinput"])
                           ] 
                )

sony_tv.actions["powertoggle"]  = IrAction([IrCode(IrEncoding_SIRC, 12, 0xA90)])
sony_tv.actions["nextinput"] = IrAction([IrCode(IrEncoding_SIRC, 12, 0xA50)])

sony_tv.actions["numeric1"] = IrAction([IrCode(IrEncoding_SIRC, 12, 0x010)])
sony_tv.actions["numeric2"] = IrAction([IrCode(IrEncoding_SIRC, 12, 0x810)])
sony_tv.actions["numeric3"] = IrAction([IrCode(IrEncoding_SIRC, 12, 0x410)])
sony_tv.actions["numeric4"] = IrAction([IrCode(IrEncoding_SIRC, 12, 0xC10)])
sony_tv.actions["numeric5"] = IrAction([IrCode(IrEncoding_SIRC, 12, 0x210)])
sony_tv.actions["numeric6"] = IrAction([IrCode(IrEncoding_SIRC, 12, 0xA10)])
sony_tv.actions["numeric7"] = IrAction([IrCode(IrEncoding_SIRC, 12, 0x610)])
sony_tv.actions["numeric8"] = IrAction([IrCode(IrEncoding_SIRC, 12, 0xE10)])
sony_tv.actions["numeric9"] = IrAction([IrCode(IrEncoding_SIRC, 12, 0x110)])
sony_tv.actions["numeric0"] = IrAction([IrCode(IrEncoding_SIRC, 12, 0x910)])

sony_tv.actions["channel_up"] 	= IrAction([IrCode(IrEncoding_SIRC, 12, 0x090)])
sony_tv.actions["channel_down"]	= IrAction([IrCode(IrEncoding_SIRC, 12, 0x890)])
sony_tv.actions["info"]		    = IrAction([IrCode(IrEncoding_SIRC, 12, 0x5D0)])

sony_tv.actions["red"] 		    = IrAction([IrCode(IrEncoding_SIRC, 15, 0x52E9)])
sony_tv.actions["yellow"] 		= IrAction([IrCode(IrEncoding_SIRC, 15, 0x72E9)])
sony_tv.actions["green"] 		= IrAction([IrCode(IrEncoding_SIRC, 15, 0x32E9)])
sony_tv.actions["blue"]	 	    = IrAction([IrCode(IrEncoding_SIRC, 15, 0x12E9)])

sony_tv.actions["guide"] 		= IrAction([IrCode(IrEncoding_SIRC, 15, 0x6D25)])
sony_tv.actions["enter"] 		= IrAction([IrCode(IrEncoding_SIRC, 12, 0xA70)])
sony_tv.actions["back"] 		= IrAction([IrCode(IrEncoding_SIRC, 12, 0xC70)])
sony_tv.actions["home"] 		= IrAction([IrCode(IrEncoding_SIRC, 12, 0x070)])

sony_tv.actions["up"] 		    = IrAction([IrCode(IrEncoding_SIRC, 12, 0x2F0)])
sony_tv.actions["down"] 		= IrAction([IrCode(IrEncoding_SIRC, 12, 0xAF0)])
sony_tv.actions["left"] 		= IrAction([IrCode(IrEncoding_SIRC, 12, 0x2D0)])
sony_tv.actions["right"] 		= IrAction([IrCode(IrEncoding_SIRC, 12, 0xCD0)])


phillips_hts = Device(
                      options = [ 
                                Option(name = "power", flags = Option_Cycled|Option_DefaultToZero|Option_ActionOnDefault, max_value = 1, change_action_names = ["powertoggle"]),
                                Option(name = "surround", flags = Option_Cycled|Option_DefaultToZero, max_value = 2, change_action_names = ["surround"])
                                ]
                     )

phillips_hts.actions["powertoggle"] = IrAction([IrCode(IrEncoding_RC6, 21, 0xFFB38), IrCode(IrEncoding_RC6, 21, 0xEFB38), 
			                                    IrCode(IrEncoding_NOP, 0, 250)])                     

phillips_hts.actions["volume_up"] 	= IrAction([IrCode(IrEncoding_RC6, 21, 0xEEFEF, 0x10000)])
phillips_hts.actions["volume_down"] = IrAction([IrCode(IrEncoding_RC6, 21, 0xEEFEE, 0x10000)])
phillips_hts.actions["mute"] 		= IrAction([IrCode(IrEncoding_RC6, 21, 0xEEFF2, 0x10000)])
phillips_hts.actions["surround"]	= IrAction([IrCode(IrEncoding_RC6, 21, 0xEEFAD, 0x10000)])

# Events
home_activity_event = Event(Event_HOME)
next_event 		    = Event(Event_NEXTPAGE)
prev_event 		    = Event(Event_PREVPAGE)
download_event      = Event(Event_DOWNLOAD)

numeric1_event 		= Event(Event_IRACTION, sony_tv.actions["numeric1"])
numeric2_event 		= Event(Event_IRACTION, sony_tv.actions["numeric2"])
numeric3_event 		= Event(Event_IRACTION, sony_tv.actions["numeric3"])
numeric4_event 		= Event(Event_IRACTION, sony_tv.actions["numeric4"])
numeric5_event 		= Event(Event_IRACTION, sony_tv.actions["numeric5"])
numeric6_event 		= Event(Event_IRACTION, sony_tv.actions["numeric6"])
numeric7_event 		= Event(Event_IRACTION, sony_tv.actions["numeric7"])
numeric8_event 		= Event(Event_IRACTION, sony_tv.actions["numeric8"])
numeric9_event 		= Event(Event_IRACTION, sony_tv.actions["numeric9"])
numeric0_event 		= Event(Event_IRACTION, sony_tv.actions["numeric0"])

volume_up_event 	= Event(Event_IRACTION, phillips_hts.actions["volume_up"])
volume_down_event 	= Event(Event_IRACTION, phillips_hts.actions["volume_down"])
mute_event 		    = Event(Event_IRACTION, phillips_hts.actions["mute"])
surround_event		= Event(Event_IRACTION, phillips_hts.actions["surround"])

channel_up_event 	= Event(Event_IRACTION, sony_tv.actions["channel_up"])
channel_down_event	= Event(Event_IRACTION, sony_tv.actions["channel_down"])
info_event		    = Event(Event_IRACTION, sony_tv.actions["info"])

red_event		    = Event(Event_IRACTION, sony_tv.actions["red"])
yellow_event		= Event(Event_IRACTION, sony_tv.actions["yellow"])
green_event		    = Event(Event_IRACTION, sony_tv.actions["green"])
blue_event 		    = Event(Event_IRACTION, sony_tv.actions["blue"])

guide_event		    = Event(Event_IRACTION, sony_tv.actions["guide"])
enter_event		    = Event(Event_IRACTION, sony_tv.actions["enter"])
back_event		    = Event(Event_IRACTION, sony_tv.actions["back"])
home_event		    = Event(Event_IRACTION, sony_tv.actions["home"])

up_event		    = Event(Event_IRACTION, sony_tv.actions["up"])
down_event		    = Event(Event_IRACTION, sony_tv.actions["down"])
left_event		    = Event(Event_IRACTION, sony_tv.actions["left"])
right_event		    = Event(Event_IRACTION, sony_tv.actions["right"])

# Activities, button mappings and touch button pages
test_remote_activity = Activity(
    [ 
	    ButtonMapping(0x200000, info_event),
	    #ButtonMapping(0x100000, blue_event),
	    #ButtonMapping(0x080000, green_event),
	    #ButtonMapping(0x040000, yellow_event),
	    #ButtonMapping(0x020000, power_event),
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
    ],
    [
        DeviceState(sony_tv, { "power": 1, "input": 0 }),
        DeviceState(phillips_hts, { "power": 1, "surround": 2 })
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
    ],
    None)

package = Package()
package.append(home_activity)
package.append(test_remote_activity)
package.append(test_remote_event)
package.append(home_activity_event)
package.append(next_event)
package.append(prev_event)
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

package.append(sony_tv)
package.append(phillips_hts)

