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
from ir import *
from ui import *
from device import *
from activity import *
from remoteconfig import *

SCREEN_WIDTH	= 240
SCREEN_HEIGHT	= 320
BUTTON_COLUMNS	= 4
BUTTON_ROWS	    = 6

BUTTON_WIDTH	= SCREEN_WIDTH/BUTTON_COLUMNS
BUTTON_HEIGHT	= SCREEN_HEIGHT/BUTTON_ROWS

BUTTON_GRID     = [
    [ 0x00400000, 0x00040000, 0x00200000, 0x00020000, 0x00100000, 0x00010000 ],
    [ 0x00000000, 0x00000001, 0x00000010, 0x00000100, 0x00001000, 0x00000000 ],
    [ 0x00000000, 0x00000002, 0x00000020, 0x00000200, 0x00002000, 0x00000000 ],
    [ 0x00000000, 0x00000004, 0x00000040, 0x00000400, 0x00004000, 0x00000000 ],
    [ 0x00000000, 0x00000008, 0x00000080, 0x00000800, 0x00008000, 0x00000000 ],
    [ 0x00800000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00080000 ],
]

remoteConfig = RemoteConfig()

sony_tv = Device("Sony TV")

sony_tv.create_action("powertoggle", [IrCode(IrEncoding_SIRC, 12, 0xA90), IrCode(IrEncoding_NOP, 0, 500)])
sony_tv.create_action("tvinput", [IrCode(IrEncoding_SIRC, 15, 0x58EE)])
sony_tv.create_action("hdmi1input", [IrCode(IrEncoding_SIRC, 15, 0x4D58)])
sony_tv.create_action("hdmi2input", [IrCode(IrEncoding_SIRC, 15, 0xCD58)])
sony_tv.create_action("hdmi3input", [IrCode(IrEncoding_SIRC, 15, 0x1D58)])
sony_tv.create_action("hdmi4input", [IrCode(IrEncoding_SIRC, 15, 0x5D58)])
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
sony_tv.create_action("options", [IrCode(IrEncoding_SIRC, 15, 0x36E9)])

sony_tv.create_action("up", [IrCode(IrEncoding_SIRC, 12, 0x2F0)])
sony_tv.create_action("down", [IrCode(IrEncoding_SIRC, 12, 0xAF0)])
sony_tv.create_action("left", [IrCode(IrEncoding_SIRC, 12, 0x2D0)])
sony_tv.create_action("right", [IrCode(IrEncoding_SIRC, 12, 0xCD0)])

sony_tv.create_action("tvpause", [IrCode(IrEncoding_SIRC, 15, 0x7358)])
sony_tv.create_action("pause", [IrCode(IrEncoding_SIRC, 15, 0x4CE9)])
sony_tv.create_action("play", [IrCode(IrEncoding_SIRC, 15, 0x2CE9)])
sony_tv.create_action("stop", [IrCode(IrEncoding_SIRC, 15, 0x0CE9)])
sony_tv.create_action("ffwd", [IrCode(IrEncoding_SIRC, 15, 0x1CE9)])
sony_tv.create_action("rewind", [IrCode(IrEncoding_SIRC, 15, 0x6CE9)])

sony_tv.create_option(name = "power", flags = Option_Cycled|Option_DefaultToZero|Option_ActionOnDefault, max_value = 1, change_action_names = ["powertoggle"])
sony_tv.create_option(name = "input", flags = Option_AlwaysSet, max_value = 4, change_action_names = ["tvinput", "hdmi1input", "hdmi2input", "hdmi3input", "hdmi4input"])

phillips_hts = Device("Phillips HTS")

phillips_hts.create_action("powertoggle", [IrCode(IrEncoding_NOP, 0, 250),
                                                IrCode(IrEncoding_RC6, 21, 0xFFB38), 
                                                IrCode(IrEncoding_RC6, 21, 0xEFB38), 
			                                    IrCode(IrEncoding_NOP, 0, 250)])                     

phillips_hts.create_action("volume_up", [IrCode(IrEncoding_RC6, 21, 0xEEFEF, 0x10000)])
phillips_hts.create_action("volume_down", [IrCode(IrEncoding_RC6, 21, 0xEEFEE, 0x10000)])
phillips_hts.create_action("mute", [IrCode(IrEncoding_RC6, 21, 0xEEFF2, 0x10000)])
phillips_hts.create_action("surround", [IrCode(IrEncoding_RC6, 21, 0xEEFAD, 0x10000), IrCode(IrEncoding_NOP, 0, 250)])
phillips_hts.create_action("source", [IrCode(IrEncoding_RC6, 21, 0xEEAC0, 0x10000)])

phillips_hts.create_option("power", Option_DefaultToZero|Option_ActionOnDefault, 1, ["powertoggle", "powertoggle"], post_delays = [0, 15000])
phillips_hts.create_option("surround", Option_Cycled|Option_DefaultToZero, 2, ["surround"], "surround")

# Note - has 8 post-data bits constant of 0x47
sony_bluray = Device("Sony Blu-ray")
sony_bluray.create_action("power-off", [IrCode(IrEncoding_SIRC, 20, 0xA8B47)])
sony_bluray.create_action("power-on", [IrCode(IrEncoding_SIRC, 20, 0xA8B47)])
sony_bluray.create_action("up", [IrCode(IrEncoding_SIRC, 20, 0x9CB47)])
sony_bluray.create_action("down", [IrCode(IrEncoding_SIRC, 20, 0x5CB47)])
sony_bluray.create_action("left", [IrCode(IrEncoding_SIRC, 20, 0xDCB47)])
sony_bluray.create_action("right", [IrCode(IrEncoding_SIRC, 20, 0x3CB47)])
sony_bluray.create_action("eject", [IrCode(IrEncoding_SIRC, 20, 0x68B47)])
sony_bluray.create_action("play", [IrCode(IrEncoding_SIRC, 20, 0x58B47)])
sony_bluray.create_action("stop", [IrCode(IrEncoding_SIRC, 20, 0x18B47)])
sony_bluray.create_action("ffwd", [IrCode(IrEncoding_SIRC, 20, 0x38B47)])
sony_bluray.create_action("rewind", [IrCode(IrEncoding_SIRC, 20, 0xD8B47)])
sony_bluray.create_option("power", Option_DefaultToZero|Option_ActionOnDefault, 1, ["power-off", "power-on"], post_delays = [5000, 25000])

sony_stereo = Device("Sony Stereo")
sony_stereo.create_action("power-on", [IrCode(IrEncoding_SIRC, 12, 0xF16)]) # Use tuner to turn on to force source to 0
sony_stereo.create_action("power-off", [IrCode(IrEncoding_SIRC, 12, 0xA81)])
sony_stereo.create_action("tuner", [IrCode(IrEncoding_SIRC, 12, 0xF16)])
sony_stereo.create_action("volume_up", [IrCode(IrEncoding_SIRC, 12, 0x481)])
sony_stereo.create_action("volume_down", [IrCode(IrEncoding_SIRC, 12, 0xC81)])
sony_stereo.create_action("function", [IrCode(IrEncoding_SIRC, 12, 0x966), IrCode(IrEncoding_NOP, 0, 125)])
sony_stereo.create_option("power", Option_DefaultToZero|Option_ActionOnDefault, 1, ["power-off", "power-on"], post_delays = [6000, 8000])
# values map to: tuner, md, cd, pc, opt, analog, tape
#sony_stereo.create_option("source", Option_Cycled|Option_AbsoluteFromZero|Option_AlwaysSet|Option_DefaultToZero, 6, ["tuner", "function"])
sony_stereo.create_option("source", Option_Cycled|Option_DefaultToZero, 6, ["function"])

remoteConfig.add_device(sony_tv)
remoteConfig.add_device(sony_bluray)
remoteConfig.add_device(sony_stereo)
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
source_event		= remoteConfig.create_ir_event("source", phillips_hts, "source")

channel_up_event 	= remoteConfig.create_ir_event("ch-up", sony_tv, "channel_up")
channel_down_event	= remoteConfig.create_ir_event("ch-down", sony_tv, "channel_down")
info_event		    = remoteConfig.create_ir_event("info", sony_tv, "info")
next_input_event    = remoteConfig.create_ir_event("next-input", sony_tv, "nextinput")

red_event		    = remoteConfig.create_ir_event("red", sony_tv, "red")
yellow_event		= remoteConfig.create_ir_event("yellow", sony_tv, "yellow")
green_event		    = remoteConfig.create_ir_event("green", sony_tv, "green")
blue_event 		    = remoteConfig.create_ir_event("blue", sony_tv, "blue")

guide_event		    = remoteConfig.create_ir_event("guide", sony_tv, "guide")
enter_event		    = remoteConfig.create_ir_event("enter", sony_tv, "enter")
back_event		    = remoteConfig.create_ir_event("back", sony_tv, "back")
home_event		    = remoteConfig.create_ir_event("tv-home", sony_tv, "home")
options_event	    = remoteConfig.create_ir_event("options", sony_tv, "options")
tv_play_event		= remoteConfig.create_ir_event("tv-play", sony_tv, "play")
tv_stop_event		= remoteConfig.create_ir_event("tv-stop", sony_tv, "stop")
tv_pause_event		= remoteConfig.create_ir_event("tv-pause", sony_tv, "tvpause")
tv_ffwd_event		= remoteConfig.create_ir_event("tv-ffwd", sony_tv, "ffwd")
tv_rewind_event		= remoteConfig.create_ir_event("tv-rewind", sony_tv, "rewind")
pause_event         = remoteConfig.create_ir_event("pause", sony_tv, "pause")

up_event		    = remoteConfig.create_ir_event("tv-up", sony_tv, "up")
down_event		    = remoteConfig.create_ir_event("tv-down", sony_tv, "down")
left_event		    = remoteConfig.create_ir_event("tv-left", sony_tv, "left")
right_event		    = remoteConfig.create_ir_event("tv-right", sony_tv, "right")

br_up_event		    = remoteConfig.create_ir_event("br-up", sony_bluray, "up")
br_down_event		= remoteConfig.create_ir_event("br-down", sony_bluray, "down")
br_left_event		= remoteConfig.create_ir_event("br-left", sony_bluray, "left")
br_right_event		= remoteConfig.create_ir_event("br-right", sony_bluray, "right")
br_eject_event		= remoteConfig.create_ir_event("br-eject", sony_bluray, "eject")
br_play_event		= remoteConfig.create_ir_event("br-play", sony_bluray, "play")
br_stop_event		= remoteConfig.create_ir_event("br-stop", sony_bluray, "stop")
br_ffwd_event		= remoteConfig.create_ir_event("br-ffwd", sony_bluray, "ffwd")
br_rewind_event		= remoteConfig.create_ir_event("br-rewind", sony_bluray, "rewind")

st_volume_up_event  = remoteConfig.create_ir_event("st-vol-up", sony_stereo, "volume_up")
st_volume_down_event  = remoteConfig.create_ir_event("st-vol-down", sony_stereo, "volume_down")

# Activities, button mappings and touch button pages
watch_tv_activity = Activity(name = "watch-tv")

watch_tv_activity.create_button_mapping(BUTTON_GRID[0][0], tv_pause_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[0][1], options_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[0][2], info_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[0][3], poweroff_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[0][4], home_activity_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[0][5], home_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[1][2], numeric1_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[1][3], numeric2_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[1][4], numeric3_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[2][2], numeric4_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[2][3], numeric5_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[2][4], numeric6_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[3][2], numeric7_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[3][3], numeric8_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[3][4], numeric9_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[4][2], mute_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[4][3], numeric0_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[4][4], surround_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[3][1], volume_up_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[4][1], volume_down_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[1][1], channel_up_event)
watch_tv_activity.create_button_mapping(BUTTON_GRID[2][1], channel_down_event)

watch_tv_activity.create_gesture_mapping(Gesture_TAP, mute_event)
watch_tv_activity.create_gesture_mapping(Gesture_DRAGLEFT, volume_up_event)
watch_tv_activity.create_gesture_mapping(Gesture_DRAGRIGHT, volume_down_event)
watch_tv_activity.create_gesture_mapping(Gesture_SWIPELEFT, channel_up_event)
watch_tv_activity.create_gesture_mapping(Gesture_SWIPERIGHT, channel_down_event)

watch_tv_activity.create_touch_button_page([
    TouchButton(guide_event, "Guide",  		      0, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(enter_event, "Enter",  BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(back_event,  "Back", 2*BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(home_event,  "Home", 3*BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),

    TouchButton(red_event,    None,  	         0, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf800, TouchButton.FLAGS_PRESS_ACTIVATE, name = "Red"),
    TouchButton(green_event,  None,   BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0x07e0, TouchButton.FLAGS_PRESS_ACTIVATE, name = "Green"),
    TouchButton(yellow_event, None, 2*BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xffe0, TouchButton.FLAGS_PRESS_ACTIVATE, name = "Yellow"),
    TouchButton(blue_event,   None, 3*BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0x001f, TouchButton.FLAGS_PRESS_ACTIVATE, name = "Blue"),

    TouchButton(up_event,    "U",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2, 2*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(down_event,  "D",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2, 3*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(left_event,  "L",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2 - BUTTON_WIDTH, int(2.5*BUTTON_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(right_event, "R",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2 + BUTTON_WIDTH, int(2.5*BUTTON_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),

    TouchButton(tv_play_event,      "Play",              0, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(tv_stop_event,      "Stop",   BUTTON_WIDTH, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(next_input_event,  "Input", 2*BUTTON_WIDTH, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(source_event,       "Srce", 3*BUTTON_WIDTH, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    ])

watch_tv_activity.create_device_state(sony_tv, { "power": 1, "input": 0 })  
watch_tv_activity.create_device_state(phillips_hts, { "power": 1, "surround": 2 })

watch_video_activity = Activity(name = "watch-video")

watch_video_activity.create_button_mapping(BUTTON_GRID[0][0], pause_event)
watch_video_activity.create_button_mapping(BUTTON_GRID[0][3], poweroff_event)
watch_video_activity.create_button_mapping(BUTTON_GRID[0][4], home_activity_event)
watch_video_activity.create_button_mapping(BUTTON_GRID[4][2], mute_event)
watch_video_activity.create_button_mapping(BUTTON_GRID[4][4], surround_event)
watch_video_activity.create_button_mapping(BUTTON_GRID[3][1], volume_up_event)
watch_video_activity.create_button_mapping(BUTTON_GRID[4][1], volume_down_event)

watch_video_activity.create_touch_button_page([
    TouchButton(guide_event, "Guide",  		      0, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(enter_event, "Enter",  BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(back_event,  "Back", 2*BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(home_event,  "Home", 3*BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),

    TouchButton(red_event,    None,  	         0, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf800, TouchButton.FLAGS_PRESS_ACTIVATE, name = "Red"),
    TouchButton(green_event,  None,   BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0x07e0, TouchButton.FLAGS_PRESS_ACTIVATE, name = "Green"),
    TouchButton(yellow_event, None, 2*BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xffe0, TouchButton.FLAGS_PRESS_ACTIVATE, name = "Yellow"),
    TouchButton(blue_event,   None, 3*BUTTON_WIDTH, BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0x001f, TouchButton.FLAGS_PRESS_ACTIVATE, name = "Blue"),

    TouchButton(up_event,    "U",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2, 2*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(down_event,  "D",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2, 3*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(left_event,  "L",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2 - BUTTON_WIDTH, int(2.5*BUTTON_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(right_event, "R",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2 + BUTTON_WIDTH, int(2.5*BUTTON_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),

    TouchButton(tv_play_event,  "Play",              0, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(tv_stop_event,  "Stop",   BUTTON_WIDTH, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(tv_rewind_event,  "<<", 2*BUTTON_WIDTH, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(tv_ffwd_event,    ">>", 3*BUTTON_WIDTH, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    ])

watch_video_activity.create_device_state(sony_tv, { "power": 1, "input": 4 })  
watch_video_activity.create_device_state(phillips_hts, { "power": 1, "surround": 2 })

watch_movie_activity = Activity(name = "watch-movie")

watch_movie_activity.create_button_mapping(BUTTON_GRID[0][0], pause_event)
watch_movie_activity.create_button_mapping(BUTTON_GRID[0][3], poweroff_event)
watch_movie_activity.create_button_mapping(BUTTON_GRID[0][4], home_activity_event)
watch_movie_activity.create_button_mapping(BUTTON_GRID[3][1], volume_up_event)
watch_movie_activity.create_button_mapping(BUTTON_GRID[4][1], volume_down_event)
watch_movie_activity.create_button_mapping(BUTTON_GRID[4][4], surround_event)
watch_movie_activity.create_button_mapping(BUTTON_GRID[4][2], mute_event)
watch_movie_activity.create_button_mapping(BUTTON_GRID[0][5], br_eject_event)

TRANSPORT_BUTTON_WIDTH  = 55
TRANSPORT_BUTTON_HEIGHT = 55
    
watch_movie_activity.create_touch_button_page([
    #TouchButton(guide_event, "Guide",  		      0, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),
    TouchButton(br_stop_event,  None, 1*BUTTON_WIDTH + ((BUTTON_WIDTH - TRANSPORT_BUTTON_WIDTH) / 2), 0, TRANSPORT_BUTTON_WIDTH, TRANSPORT_BUTTON_HEIGHT, 0x0000, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_NO_BORDER, "Resources/stop-button-3.png", "Resources/stop-button-3-pressed.png"),
    TouchButton(br_play_event,  None, 2*BUTTON_WIDTH + ((BUTTON_WIDTH - TRANSPORT_BUTTON_WIDTH) / 2), 0, TRANSPORT_BUTTON_WIDTH, TRANSPORT_BUTTON_HEIGHT, 0x0000, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_NO_BORDER, "Resources/play-button-3.png", "Resources/play-button-3-pressed.png"),
    #TouchButton(home_event,  "Home", 3*BUTTON_WIDTH, 0, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, True, True),

    TouchButton(br_up_event,    "U",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2, 2*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(br_down_event,  "D",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2, 3*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(br_left_event,  "L",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2 - BUTTON_WIDTH, int(2.5*BUTTON_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(br_right_event, "R",  (SCREEN_WIDTH - BUTTON_WIDTH) / 2 + BUTTON_WIDTH, int(2.5*BUTTON_HEIGHT), BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),

    TouchButton(enter_event,   "Enter", (SCREEN_WIDTH - BUTTON_WIDTH) / 2 - BUTTON_WIDTH, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(br_rewind_event,  "<<", (SCREEN_WIDTH - BUTTON_WIDTH) / 2,                4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    TouchButton(br_ffwd_event,    ">>", (SCREEN_WIDTH - BUTTON_WIDTH) / 2 + BUTTON_WIDTH, 4*BUTTON_HEIGHT, BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_PRESS_ACTIVATE|TouchButton.FLAGS_CENTRE_TEXT),
    ])
	    
watch_movie_activity.create_device_state(sony_tv, { "power": 1, "input": 1 })
watch_movie_activity.create_device_state(sony_bluray, { "power": 1 })
watch_movie_activity.create_device_state(phillips_hts, { "power": 1, "surround": 2 })

listen_cd_activity = Activity(name = "listen-cd")

listen_cd_activity.create_button_mapping(BUTTON_GRID[0][3], poweroff_event)
listen_cd_activity.create_button_mapping(BUTTON_GRID[0][4], home_activity_event)
listen_cd_activity.create_button_mapping(BUTTON_GRID[3][1], st_volume_up_event)
listen_cd_activity.create_button_mapping(BUTTON_GRID[4][1], st_volume_down_event)
    
listen_cd_activity.create_device_state(sony_stereo, { "power": 1, "source": 2 })

listen_radio_activity = Activity(name = "listen-radio")

listen_radio_activity.create_button_mapping(BUTTON_GRID[0][3], poweroff_event)
listen_radio_activity.create_button_mapping(BUTTON_GRID[0][4], home_activity_event)
listen_radio_activity.create_button_mapping(BUTTON_GRID[3][1], st_volume_up_event)
listen_radio_activity.create_button_mapping(BUTTON_GRID[4][1], st_volume_down_event)
    
listen_radio_activity.create_device_state(sony_stereo, { "power": 1, "source": 0 })

watch_tv_event = remoteConfig.create_activity_event("watch-tv", watch_tv_activity)
watch_video_event = remoteConfig.create_activity_event("watch-video", watch_video_activity)
watch_movie_event = remoteConfig.create_activity_event("watch-movie", watch_movie_activity)
listen_cd_event = remoteConfig.create_activity_event("listen-cd", listen_cd_activity)
listen_radio_event = remoteConfig.create_activity_event("listen-radio", listen_radio_activity)

home_activity = Activity(name = "home", flags = Activity_NoDevices)

home_activity.create_button_mapping(BUTTON_GRID[0][0], prev_event)
home_activity.create_button_mapping(BUTTON_GRID[0][5], next_event)
home_activity.create_button_mapping(BUTTON_GRID[0][3], poweroff_event)

home_activity.create_touch_button_page([
    TouchButton(watch_tv_event, "Watch TV", 0, 0*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_CENTRE_TEXT),   
    TouchButton(watch_video_event, "Watch Video", 0, 1*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_CENTRE_TEXT),   
    TouchButton(watch_movie_event, "Watch Movie", 0, 2*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_CENTRE_TEXT),   
    TouchButton(listen_cd_event, "Listen to CD", 0, 3*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_CENTRE_TEXT),   
    TouchButton(listen_radio_event, "Listen to radio", 0, 4*BUTTON_HEIGHT, 4*BUTTON_WIDTH, BUTTON_HEIGHT, 0xf9e0, TouchButton.FLAGS_CENTRE_TEXT),   
    ])

remoteConfig.add_activity(home_activity)
remoteConfig.add_activity(watch_tv_activity)
remoteConfig.add_activity(watch_video_activity)
remoteConfig.add_activity(watch_movie_activity)
remoteConfig.add_activity(listen_cd_activity)
remoteConfig.add_activity(listen_radio_activity)
remoteConfig.set_home_activity(home_activity)

package = Package()
package.append(remoteConfig)

