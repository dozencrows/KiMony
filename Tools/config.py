#!/usr/bin/python

#=======================================================================
# Copyright Nicholas Tuckett 2015.
# Distributed under the MIT License.
# (See accompanying file license.txt or copy at
#  http://opensource.org/licenses/MIT)
#=======================================================================

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

sony_tv = Device()

sony_tv.options_list = [ 
                       Option(name = "power", flags = Option_Cycled|Option_DefaultToZero|Option_ActionOnDefault, max_value = 1, change_action_names = ["powertoggle"]),
                       Option(name = "input", flags = Option_Cycled, max_value = 5, change_action_names = ["nextinput"])
                       ] 

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


phillips_hts = Device()

phillips_hts.actions["powertoggle"] = IrAction([IrCode(IrEncoding_NOP, 0, 250),
                                                IrCode(IrEncoding_RC6, 21, 0xFFB38), 
                                                IrCode(IrEncoding_RC6, 21, 0xEFB38), 
			                                    IrCode(IrEncoding_NOP, 0, 250)])                     

phillips_hts.actions["volume_up"] 	= IrAction([IrCode(IrEncoding_RC6, 21, 0xEEFEF, 0x10000)])
phillips_hts.actions["volume_down"] = IrAction([IrCode(IrEncoding_RC6, 21, 0xEEFEE, 0x10000)])
phillips_hts.actions["mute"] 		= IrAction([IrCode(IrEncoding_RC6, 21, 0xEEFF2, 0x10000)])
phillips_hts.actions["surround"]	    = IrAction([IrCode(IrEncoding_RC6, 21, 0xEEFAD, 0x10000), IrCode(IrEncoding_NOP, 0, 250)])
phillips_hts.actions["pre-surround"]	= IrAction([IrCode(IrEncoding_NOP, 0, 20000), IrCode(IrEncoding_RC6, 21, 0xEEFAD, 0x10000), IrCode(IrEncoding_NOP, 0, 250)])

phillips_hts.options_list = [ 
                            Option(name = "power", flags = Option_Cycled|Option_DefaultToZero|Option_ActionOnDefault, max_value = 1, 
                                     change_action_names = ["powertoggle"]),
                            Option(name = "surround", flags = Option_Cycled|Option_DefaultToZero, max_value = 2, 
                                     pre_action = phillips_hts.actions['pre-surround'], change_action_names = ["surround"])
                            ]

sony_bluray = Device()
sony_bluray.actions["powertoggle"]  = IrAction([IrCode(IrEncoding_SIRC, 20, 0xA8B47)])
sony_bluray.actions["up"] 		    = IrAction([IrCode(IrEncoding_SIRC, 20, 0x9CB47)])
sony_bluray.actions["down"] 		= IrAction([IrCode(IrEncoding_SIRC, 20, 0x5CB47)])
sony_bluray.actions["left"] 		= IrAction([IrCode(IrEncoding_SIRC, 20, 0xDCB47)])
sony_bluray.actions["right"] 		= IrAction([IrCode(IrEncoding_SIRC, 20, 0x3CB47)])

sony_bluray.options_list = [
                            Option(name = "power", flags = Option_Cycled|Option_DefaultToZero|Option_ActionOnDefault, max_value = 1, 
                                     change_action_names = ["powertoggle"])
                           ]

#  post_data_bits  8
#'post_data      0x47
#          KEY_EJECTCD              0x68B
#          KEY_POWER                0xA8B
#          KEY_1                    0x00B
#          KEY_2                    0x80B
#          KEY_3                    0x40B
#          KEY_4                    0xC0B
#          KEY_5                    0x20B
#          KEY_6                    0xA0B
#          KEY_7                    0x60B
#          KEY_8                    0xE0B
#          KEY_9                    0x10B
#          KEY_0                    0x90B
#          KEY_AUDIO                0x26B
#          KEY_SUBTITLE             0xC6B
#          KEY_RED                  0xE6B
#          KEY_GREEN                0x16B
#          KEY_BLUE                 0x66B
#          KEY_YELLOW               0x96B
#          KEY_MENU                 0x34B
#          KEY_CONTEXT_MENU         0x94B
#          KEY_UP                   0x9CB
#          KEY_DOWN                 0x5CB
#          KEY_LEFT                 0xDCB
#          KEY_RIGHT                0x3CB
#          KEY_OK                   0xBCB
#          KEY_BACK                 0xC2B
#          KEY_OPTION               0xFCB
#          KEY_HOME                 0x42B
#          KEY_RESTART              0xEAB
#          KEY_PAUSE                0x98B
#          KEY_END                  0x6AB
#          KEY_REWIND               0xD8B
#          KEY_PLAY                 0x58B
#          KEY_FASTFORWARD          0x38B
#          KEY_DISPLAYTOGGLE        0x82B
#          KEY_STOP                 0x18B
#          KEY_WWW                  0x32B


# Events
home_activity_event = Event(Event_HOME)
next_event 		    = Event(Event_NEXTPAGE)
prev_event 		    = Event(Event_PREVPAGE)
download_event      = Event(Event_DOWNLOAD)
poweroff_event      = Event(Event_POWEROFF)

numeric1_event 		= Event(Event_IRACTION, [sony_tv.actions["numeric1"], sony_tv])
numeric2_event 		= Event(Event_IRACTION, [sony_tv.actions["numeric2"], sony_tv])
numeric3_event 		= Event(Event_IRACTION, [sony_tv.actions["numeric3"], sony_tv])
numeric4_event 		= Event(Event_IRACTION, [sony_tv.actions["numeric4"], sony_tv])
numeric5_event 		= Event(Event_IRACTION, [sony_tv.actions["numeric5"], sony_tv])
numeric6_event 		= Event(Event_IRACTION, [sony_tv.actions["numeric6"], sony_tv])
numeric7_event 		= Event(Event_IRACTION, [sony_tv.actions["numeric7"], sony_tv])
numeric8_event 		= Event(Event_IRACTION, [sony_tv.actions["numeric8"], sony_tv])
numeric9_event 		= Event(Event_IRACTION, [sony_tv.actions["numeric9"], sony_tv])
numeric0_event 		= Event(Event_IRACTION, [sony_tv.actions["numeric0"], sony_tv])

volume_up_event 	= Event(Event_IRACTION, [phillips_hts.actions["volume_up"], phillips_hts])
volume_down_event 	= Event(Event_IRACTION, [phillips_hts.actions["volume_down"], phillips_hts])
mute_event 		    = Event(Event_IRACTION, [phillips_hts.actions["mute"], phillips_hts])
surround_event		= Event(Event_IRACTION, [phillips_hts.actions["surround"], phillips_hts])

channel_up_event 	= Event(Event_IRACTION, [sony_tv.actions["channel_up"], sony_tv])
channel_down_event	= Event(Event_IRACTION, [sony_tv.actions["channel_down"], sony_tv])
info_event		    = Event(Event_IRACTION, [sony_tv.actions["info"], sony_tv])

red_event		    = Event(Event_IRACTION, [sony_tv.actions["red"], sony_tv])
yellow_event		= Event(Event_IRACTION, [sony_tv.actions["yellow"], sony_tv])
green_event		    = Event(Event_IRACTION, [sony_tv.actions["green"], sony_tv])
blue_event 		    = Event(Event_IRACTION, [sony_tv.actions["blue"], sony_tv])

guide_event		    = Event(Event_IRACTION, [sony_tv.actions["guide"], sony_tv])
enter_event		    = Event(Event_IRACTION, [sony_tv.actions["enter"], sony_tv])
back_event		    = Event(Event_IRACTION, [sony_tv.actions["back"], sony_tv])
home_event		    = Event(Event_IRACTION, [sony_tv.actions["home"], sony_tv])

up_event		    = Event(Event_IRACTION, [sony_tv.actions["up"], sony_tv])
down_event		    = Event(Event_IRACTION, [sony_tv.actions["down"], sony_tv])
left_event		    = Event(Event_IRACTION, [sony_tv.actions["left"], sony_tv])
right_event		    = Event(Event_IRACTION, [sony_tv.actions["right"], sony_tv])

br_power_event      = sony_bluray.create_event("powertoggle")
br_up_event		    = sony_bluray.create_event("up")
br_down_event		= sony_bluray.create_event("down")
br_left_event		= sony_bluray.create_event("left")
br_right_event		= sony_bluray.create_event("right")

# Activities, button mappings and touch button pages
watch_tv_activity = Activity(
    [ 
	    ButtonMapping(0x200000, info_event),
	    #ButtonMapping(0x100000, blue_event),
	    #ButtonMapping(0x080000, green_event),
	    #ButtonMapping(0x040000, yellow_event),
	    ButtonMapping(0x020000, poweroff_event),
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
    

watch_movie_activity = Activity(
    [ 
	    #ButtonMapping(0x200000, info_event),
	    #ButtonMapping(0x100000, blue_event),
	    #ButtonMapping(0x080000, green_event),
	    #ButtonMapping(0x040000, yellow_event),
	    ButtonMapping(0x020000, poweroff_event),
	    ButtonMapping(0x010000, home_activity_event),
	    #ButtonMapping(0x008000, numeric1_event),
	    #ButtonMapping(0x000800, numeric2_event),
	    #ButtonMapping(0x000080, numeric3_event),
	    ButtonMapping(0x000008, volume_up_event),
	    #ButtonMapping(0x004000, numeric4_event),
	    #ButtonMapping(0x000400, numeric5_event),
	    #ButtonMapping(0x000040, numeric6_event),
	    ButtonMapping(0x000004, volume_down_event),
	    #ButtonMapping(0x002000, numeric7_event),
	    #ButtonMapping(0x000200, numeric8_event),
	    #ButtonMapping(0x000020, numeric9_event),
	    #ButtonMapping(0x000002, channel_up_event),
	    ButtonMapping(0x001000, surround_event),
	    #ButtonMapping(0x000100, numeric0_event),
	    ButtonMapping(0x000010, mute_event),
	    #ButtonMapping(0x000001, channel_down_event),
    ],

    [
        TouchButtonPage(
        [
	        TouchButton(br_up_event, "U",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2, 2*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),
	        TouchButton(br_down_event, "D",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2, 3*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),
	        TouchButton(br_left_event, "L",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2 - BUTTON_WIDTH, int(2.5*BUTTON_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),
	        TouchButton(br_right_event, "R",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2 + BUTTON_WIDTH, int(2.5*BUTTON_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),
	    ])
	],
    [
        DeviceState(sony_tv, { "power": 1, "input": 0 }),
        DeviceState(sony_bluray, { "power": 1 }),
        DeviceState(phillips_hts, { "power": 1, "surround": 2 }),
    ])

watch_tv_event = Event(Event_ACTIVITY, [watch_tv_activity])
watch_movie_event = Event(Event_ACTIVITY, [watch_movie_activity])

home_activity = Activity(
    [ ButtonMapping(0x010000, prev_event), ButtonMapping(0x040000, next_event), ButtonMapping(0x100000, download_event), ButtonMapping(0x020000, poweroff_event) ],
    [ 
        TouchButtonPage(
        [
            TouchButton(watch_tv_event, "Watch TV", 0, 0*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, False, True),   
            TouchButton(watch_movie_event, "Watch Movie", 0, 1*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, False, True),   
        ]),
    ],
    None,
    flags = Activity_NoDevices
    )

header = RemoteDataHeader(
    home_activity,
    [ sony_tv, sony_bluray, phillips_hts ],
    [ home_activity, watch_tv_activity, watch_movie_activity ]
    )
    
package = Package()
package.append(header)
package.append(home_activity_event)
package.append(watch_tv_event)
package.append(watch_movie_event)
package.append(next_event)
package.append(prev_event)
package.append(download_event)
package.append(poweroff_event)
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
package.append(br_power_event)
package.append(br_up_event)
package.append(br_down_event)
package.append(br_left_event)
package.append(br_right_event)

