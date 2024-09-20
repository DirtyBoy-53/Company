import sys
import argparse
import os
import gi

gi.require_version('Gst', '1.0')
from gi.repository import Gst, GObject, GLib

def video_convert_avi2mp4(file_in):
    filename = (os.path.splitext(video_file)[0])
    filetype = (os.path.splitext(video_file)[1])
    file_out = filename + '.mp4'

    if filetype != '.avi':
        print("Wrong File Type ! Must be .avi !")
        return
    
    launch_str = ("filesrc location=%s ! avidemux name=demux ! video/x-h264 ! h264parse ! qtmux ! filesink location=%s") % (video_file, file_out)
    print(launch_str)

    Gst.init()
    pipeline = Gst.parse_launch(launch_str)
    pipeline.set_state(Gst.State.PLAYING)

    # wait until EOS or error
    bus = pipeline.get_bus()
    msg = bus.timed_pop_filtered(
        Gst.CLOCK_TIME_NONE,
        Gst.MessageType.ERROR | Gst.MessageType.EOS
    )
    # free resources
    pipeline.set_state(Gst.State.NULL)

def video_play_avi_h_264(file_in):
    filetype = (os.path.splitext(video_file)[1])
    print(filetype)

    if filetype != '.avi':
        print("Wrong File Type ! Must be .avi !")
        return
    
    launch_str = ("filesrc location=%s ! avidemux name=demux ! video/x-h264 ! h264parse ! avdec_h264 ! autovideosink sync=false") % (video_file)
    print(launch_str)

    Gst.init()
    pipeline = Gst.parse_launch(launch_str)
    pipeline.set_state(Gst.State.PLAYING)

    # wait until EOS or error
    bus = pipeline.get_bus()
    msg = bus.timed_pop_filtered(
        Gst.CLOCK_TIME_NONE,
        Gst.MessageType.ERROR | Gst.MessageType.EOS
    )
    # free resources
    pipeline.set_state(Gst.State.NULL)

def video_avimux_h_264(file_in):
    filename = (os.path.splitext(video_file)[0])
    filetype = (os.path.splitext(video_file)[1])
    file_out = filename + '.mp4'
    print(file_out)

    if filetype != '.h264':
        print("Wrong File Type ! Must be .avi !")
        return
    
    launch_str = ("filesrc location=%s ! h264parse ! avimux ! filesink location=%s") % (video_file, file_out)
    print(launch_str)

    Gst.init()
    pipeline = Gst.parse_launch(launch_str)
    pipeline.set_state(Gst.State.PLAYING)

    # wait until EOS or error
    bus = pipeline.get_bus()
    msg = bus.timed_pop_filtered(
        Gst.CLOCK_TIME_NONE,
        Gst.MessageType.ERROR | Gst.MessageType.EOS
    )
    # free resources
    pipeline.set_state(Gst.State.NULL)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Video Convert by Gst"
    )
    parser.add_argument('--file_in',
                        type=str,
                        help="video file input (h264.avi format)",
                        required=True
    )
    parser.add_argument('--method',
                        type=str,
                        help="convert | play | avimux)",
                        required=True
    )

    args = parser.parse_args()
    video_file = args.file_in
    method = args.method

    if method == 'convert':
        video_convert_avi2mp4(video_file)
    elif method == 'play':
        video_play_avi_h_264(video_file)
    elif method == 'avimux':
        video_avimux_h_264(video_file)
    else:
        print("Wrong input method!")
