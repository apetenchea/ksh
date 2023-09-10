import argparse
import ctypes
import psutil

from driver import CtlCode, device_io_ctl, format_dos_device, encode_string


def test():
    input_buffer = ctypes.c_ulong(1)
    device_io_ctl(
        CtlCode.IOCTL_KSH_TEST, ctypes.byref(input_buffer), ctypes.sizeof(input_buffer)
    )


def pkill(pid):
    input_buffer = ctypes.c_ulong(pid)
    device_io_ctl(
        CtlCode.IOCTL_KSH_PKILL, ctypes.byref(input_buffer), ctypes.sizeof(input_buffer)
    )


def rm(path):
    path = format_dos_device(path)
    input_buffer = ctypes.create_unicode_buffer(path)
    device_io_ctl(
        CtlCode.IOCTL_KSH_REMOVE_FILE, input_buffer, ctypes.sizeof(input_buffer)
    )


def cp(source, dest):
    source = encode_string(format_dos_device(source))
    dest = encode_string(format_dos_device(dest))
    input_buffer = ctypes.create_string_buffer(source + dest)
    device_io_ctl(
        CtlCode.IOCTL_KSH_COPY_FILE, input_buffer, ctypes.sizeof(input_buffer)
    )


def mv(source, dest):
    source = encode_string(format_dos_device(source))
    dest = encode_string(format_dos_device(dest))
    input_buffer = ctypes.create_string_buffer(source + dest)
    device_io_ctl(
        CtlCode.IOCTL_KSH_MOVE_FILE, input_buffer, ctypes.sizeof(input_buffer)
    )


def main(args):
    if args.command == "test":
        test()
    elif args.command == "pkill":
        if args.name:
            for proc in psutil.process_iter():
                if proc.name() == args.name:
                    args.pid = proc.pid
                    break
        if args.pid is None:
            raise Exception("Process not found")
        pkill(args.pid)
    elif args.command == "rm":
        rm(args.path)
    elif args.command == "cp":
        cp(args.source, args.destination)
    elif args.command == "mv":
        mv(args.source, args.destination)
    else:
        raise Exception("Invalid command")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="ksh", description="Kernel Shell", epilog="Use with caution"
    )
    subparsers = parser.add_subparsers(dest="command", help="Command to be executed")

    parser_test = subparsers.add_parser("test", help="Test if the driver is working")

    parser_pkill = subparsers.add_parser("pkill", help="Kill a process by PID or name")
    group_pkill = parser_pkill.add_mutually_exclusive_group(required=True)
    group_pkill.add_argument("-p", "--pid", type=int, help="PID of the process to kill")
    group_pkill.add_argument(
        "-n", "--name", type=str, help="Name of the process to kill"
    )

    parser_rm = subparsers.add_parser("rm", help="Remove a file")
    parser_rm.add_argument("path", type=str, help="Path of the file to remove")

    parser_cp = subparsers.add_parser("cp", help="Copy a file")
    parser_cp.add_argument("source", type=str, help="Path of the file to copy")
    parser_cp.add_argument("destination", type=str, help="Path of the destination file")

    parser_mv = subparsers.add_parser("mv", help="Move a file")
    parser_mv.add_argument("source", type=str, help="Path of the file to move")
    parser_mv.add_argument("destination", type=str, help="Path of the destination file")

    main(parser.parse_args())
