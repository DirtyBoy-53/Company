import argparse
import algSDKpy
from algSDKpy import service_debug_control

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Set debug on/off"
    )
    parser.add_argument('--status',
                        type=int,
                        help="debug flag param 0:close 1:open)",
                        required=True
    )
    parser.add_argument('-time_out',
                        type=int,
                        help="Timeout value for request (in milliseconds)",
                        default=5000
    )
    args = parser.parse_args()
    status = args.status
    timeo = args.time_out

    cmd_topic = b"/service/debug/set"
    debug_ctl = service_debug_control()
    debug_ctl.ack_mode = 1
    debug_ctl.debug_flag = status

    ret = algSDKpy.CallServices(cmd_topic, debug_ctl, timeo)
    print(' result = %d ' % ret)

    print('---------finish-------------')
