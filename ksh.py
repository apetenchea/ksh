import argparse
import ctypes
import psutil

from driver import CtlCode, device_io_ctl


def test():
    input_buffer = ctypes.c_ulong(1)
    device_io_ctl(CtlCode.IOCTL_KSH_TEST, input_buffer)


def pkill(pid):
    input_buffer = ctypes.c_ulong(pid)
    device_io_ctl(CtlCode.IOCTL_KSH_PKILL, input_buffer)


def rm(path):
    if not path.startswith("\\"):
        path = r"\DosDevices\{}".format(path)
    input_buffer = ctypes.create_unicode_buffer(path, len(path))
    device_io_ctl(CtlCode.IOCTL_KSH_REMOVE_FILE, input_buffer)


def cp(source, dest):
    if not source.startswith("\\"):
        source = r"\DosDevices\{}".format(source)
    if not dest.startswith("\\"):
        dest = r"\DosDevices\{}".format(dest)
    merged = f"{source}|{dest}"
    input_buffer = ctypes.create_unicode_buffer(merged, len(merged))
    print(input_buffer.value)
    device_io_ctl(CtlCode.IOCTL_KSH_COPY_FILE, input_buffer)


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

    main(parser.parse_args())
