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

remoteConfig = RemoteConfig()

sony_tv = Device("Sony TV")

sony_tv.options_list = [ 
                       Option(name = "power", flags = Option_Cycled|Option_DefaultToZero|Option_ActionOnDefault, max_value = 1, change_action_names = ["powertoggle"]),
                       Option(name = "input", flags = Option_Cycled, max_value = 5, change_action_names = ["nextinput"])
                       ] 

sony_tv.create_action("powertoggle", [IrCode(IrEncoding_SIRC, 12, 0xA90)])

sony_tv.create_action("nextinput", [IrCode(IrEncoding_SIRC, 12, 0xA50)])

sony_tv.create_action("numeric1", [IrCode(IrEncoding_SIRC, 12, 0x010)])
sony_tv.create_action("numeric2", [IrCode(IrEncoding_SIRC, 12, 0x810)])
sony_tv.create_action("numeric3", [IrCode(IrEncoding_SIRC, 12, 0x410)])
sony_tv.create_action("numeric4", [IrCode(IrEncoding_SIRC, 12, 0xC10)])
sony_tv.create_action("numeric5", [IrCode(IrEncoding_SIRC, 12, 0x210)])
sony_tv.create_action("numeric6", [IrCode(IrEncoding_SIRC, 12, 0xA10)])
sony_tv.create_action("numeric7", [IrCode(IrEncoding_SIRC, 12, 0x610)])
sony_tv.create_action("numeric8", [IrCode(IrEncoding_SIRC, 12, 0xE10)])
sony_tv.create_action("numeric9", [IrCode(IrEncoding_SIRC, 12, 0x110)])
sony_tv.create_action("numeric0", [IrCode(IrEncoding_SIRC, 12, 0x910)])

sony_tv.create_action("channel_up", [IrCode(IrEncoding_SIRC, 12, 0x090)])
sony_tv.create_action("channel_down", [IrCode(IrEncoding_SIRC, 12, 0x890)])
sony_tv.create_action("info", [IrCode(IrEncoding_SIRC, 12, 0x5D0)])

sony_tv.create_action("red", [IrCode(IrEncoding_SIRC, 15, 0x52E9)])
sony_tv.create_action("yellow", [IrCode(IrEncoding_SIRC, 15, 0x72E9)])
sony_tv.create_action("green", [IrCode(IrEncoding_SIRC, 15, 0x32E9)])
sony_tv.create_action("blue", [IrCode(IrEncoding_SIRC, 15, 0x12E9)])

sony_tv.create_action("guide", [IrCode(IrEncoding_SIRC, 15, 0x6D25)])
sony_tv.create_action("enter", [IrCode(IrEncoding_SIRC, 12, 0xA70)])
sony_tv.create_action("back", [IrCode(IrEncoding_SIRC, 12, 0xC70)])
sony_tv.create_action("home", [IrCode(IrEncoding_SIRC, 12, 0x070)])

sony_tv.create_action("up", [IrCode(IrEncoding_SIRC, 12, 0x2F0)])
sony_tv.create_action("down", [IrCode(IrEncoding_SIRC, 12, 0xAF0)])
sony_tv.create_action("left", [IrCode(IrEncoding_SIRC, 12, 0x2D0)])
sony_tv.create_action("right", [IrCode(IrEncoding_SIRC, 12, 0xCD0)])


phillips_hts = Device("Phillips HTS")

phillips_hts.create_action("powertoggle", [IrCode(IrEncoding_NOP, 0, 250),
                                                IrCode(IrEncoding_RC6, 21, 0xFFB38), 
                                                IrCode(IrEncoding_RC6, 21, 0xEFB38), 
			                                    IrCode(IrEncoding_NOP, 0, 250)])                     

phillips_hts.create_action("volume_up", [IrCode(IrEncoding_RC6, 21, 0xEEFEF, 0x10000)])
phillips_hts.create_action("volume_down", [IrCode(IrEncoding_RC6, 21, 0xEEFEE, 0x10000)])
phillips_hts.create_action("mute", [IrCode(IrEncoding_RC6, 21, 0xEEFF2, 0x10000)])
phillips_hts.create_action("surround", [IrCode(IrEncoding_RC6, 21, 0xEEFAD, 0x10000), IrCode(IrEncoding_NOP, 0, 250)])
phillips_hts.create_action("pre-surround", [IrCode(IrEncoding_NOP, 0, 20000), IrCode(IrEncoding_RC6, 21, 0xEEFAD, 0x10000), IrCode(IrEncoding_NOP, 0, 250)])

phillips_hts.options_list = [ 
                            Option(name = "power", flags = Option_Cycled|Option_DefaultToZero|Option_ActionOnDefault, max_value = 1, 
                                     change_action_names = ["powertoggle"]),
                            Option(name = "surround", flags = Option_Cycled|Option_DefaultToZero, max_value = 2, 
                                     pre_action = phillips_hts.actions['pre-surround'], change_action_names = ["surround"])
                            ]

sony_bluray = Device("Sony Blu-ray")
sony_bluray.create_action("powertoggle", [IrCode(IrEncoding_SIRC, 20, 0xA8B47)])
sony_bluray.create_action("up", [IrCode(IrEncoding_SIRC, 20, 0x9CB47)])
sony_bluray.create_action("down", [IrCode(IrEncoding_SIRC, 20, 0x5CB47)])
sony_bluray.create_action("left", [IrCode(IrEncoding_SIRC, 20, 0xDCB47)])
sony_bluray.create_action("right", [IrCode(IrEncoding_SIRC, 20, 0x3CB47)])

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


remoteConfig.add_device(sony_tv)
remoteConfig.add_device(sony_bluray)
remoteConfig.add_device(phillips_hts)

# Events
home_activity_event = remoteConfig.create_event("home",         Event_HOME)
next_event 		    = remoteConfig.create_event("next",         Event_NEXTPAGE)
prev_event 		    = remoteConfig.create_event("prev",         Event_PREVPAGE)
download_event      = remoteConfig.create_event("download",     Event_DOWNLOAD)
poweroff_event      = remoteConfig.create_event("power-off",    Event_POWEROFF)

numeric1_event 		= remoteConfig.create_ir_event("tv-1", sony_tv, "numeric1")
numeric2_event 		= remoteConfig.create_ir_event("tv-2", sony_tv, "numeric2")
numeric3_event 		= remoteConfig.create_ir_event("tv-3", sony_tv, "numeric3")
numeric4_event 		= remoteConfig.create_ir_event("tv-4", sony_tv, "numeric4")
numeric5_event 		= remoteConfig.create_ir_event("tv-5", sony_tv, "numeric5")
numeric6_event 		= remoteConfig.create_ir_event("tv-6", sony_tv, "numeric6")
numeric7_event 		= remoteConfig.create_ir_event("tv-7", sony_tv, "numeric7")
numeric8_event 		= remoteConfig.create_ir_event("tv-8", sony_tv, "numeric8")
numeric9_event 		= remoteConfig.create_ir_event("tv-9", sony_tv, "numeric9")
numeric0_event 		= remoteConfig.create_ir_event("tv-0", sony_tv, "numeric0")

volume_up_event 	= remoteConfig.create_ir_event("vol-up", phillips_hts, "volume_up")
volume_down_event 	= remoteConfig.create_ir_event("vol-down", phillips_hts, "volume_down")
mute_event 		    = remoteConfig.create_ir_event("mute", phillips_hts, "mute")
surround_event		= remoteConfig.create_ir_event("surround", phillips_hts, "surround")

channel_up_event 	= remoteConfig.create_ir_event("ch-up", sony_tv, "channel_up")
channel_down_event	= remoteConfig.create_ir_event("ch-down", sony_tv, "channel_down")
info_event		    = remoteConfig.create_ir_event("info", sony_tv, "info")

red_event		    = remoteConfig.create_ir_event("red", sony_tv, "red")
yellow_event		= remoteConfig.create_ir_event("yellow", sony_tv, "yellow")
green_event		    = remoteConfig.create_ir_event("green", sony_tv, "green")
blue_event 		    = remoteConfig.create_ir_event("blue", sony_tv, "blue")

guide_event		    = remoteConfig.create_ir_event("guide", sony_tv, "guide")
enter_event		    = remoteConfig.create_ir_event("enter", sony_tv, "enter")
back_event		    = remoteConfig.create_ir_event("back", sony_tv, "back")
home_event		    = remoteConfig.create_ir_event("tv-home", sony_tv, "home")

up_event		    = remoteConfig.create_ir_event("tv-up", sony_tv, "up")
down_event		    = remoteConfig.create_ir_event("tv-down", sony_tv, "down")
left_event		    = remoteConfig.create_ir_event("tv-left", sony_tv, "left")
right_event		    = remoteConfig.create_ir_event("tv-right", sony_tv, "right")

br_power_event      = remoteConfig.create_ir_event("br-power", sony_bluray, "powertoggle")
br_up_event		    = remoteConfig.create_ir_event("br-up", sony_bluray, "up")
br_down_event		= remoteConfig.create_ir_event("br-down", sony_bluray, "down")
br_left_event		= remoteConfig.create_ir_event("br-left", sony_bluray, "left")
br_right_event		= remoteConfig.create_ir_event("br-right", sony_bluray, "right")

# Activities, button mappings and touch button pages
watch_tv_activity = Activity(name = "watch-tv",
    button_mappings = [ 
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
    touch_button_pages = [
        TouchButtonPage(name = "1",
            touch_buttons = [
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
        ])
    ]
    )

watch_tv_activity.create_device_state(sony_tv, { "power": 1, "input": 0 })  
watch_tv_activity.create_device_state(phillips_hts, { "power": 1, "surround": 2 })

watch_movie_activity = Activity(name = "watch-movie",
    button_mappings = [ 
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

    touch_button_pages = [
        TouchButtonPage(name = '1',
            touch_buttons = [
	            TouchButton(br_up_event, "U",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2, 2*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),
	            TouchButton(br_down_event, "D",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2, 3*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),
	            TouchButton(br_left_event, "L",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2 - BUTTON_WIDTH, int(2.5*BUTTON_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),
	            TouchButton(br_right_event, "R",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2 + BUTTON_WIDTH, int(2.5*BUTTON_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),
	    ])
	]
    )
    
watch_movie_activity.create_device_state(sony_tv, { "power": 1, "input": 0 })
watch_movie_activity.create_device_state(sony_bluray, { "power": 1 })
watch_movie_activity.create_device_state(phillips_hts, { "power": 1, "surround": 2 })

watch_tv_event = remoteConfig.create_activity_event("watch-tv", watch_tv_activity)
watch_movie_event = remoteConfig.create_activity_event("watch-movie", watch_movie_activity)

home_activity = Activity(name = "home",
    button_mappings = [ ButtonMapping(0x010000, prev_event), ButtonMapping(0x040000, next_event), ButtonMapping(0x100000, download_event), ButtonMapping(0x020000, poweroff_event) ],
    touch_button_pages = [ 
        TouchButtonPage(name = '1',
            touch_buttons = [
                TouchButton(watch_tv_event, "Watch TV", 0, 0*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, False, True),   
                TouchButton(watch_movie_event, "Watch Movie", 0, 1*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, False, True),   
        ]),
    ],
    flags = Activity_NoDevices
    )

remoteConfig.add_activity(home_activity)
remoteConfig.add_activity(watch_tv_activity)
remoteConfig.add_activity(watch_movie_activity)
remoteConfig.set_home_activity(home_activity)

package = Package()
package.append(remoteConfig)

