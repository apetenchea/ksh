import ctypes
from enum import IntEnum

import win32api
import win32file


class CtlCode(IntEnum):
    IOCTL_KSH_PKILL = 0x800
    IOCTL_KSH_TEST = 0x801
    IOCTL_KSH_REMOVE_FILE = 0x802
    IOCTL_KSH_COPY_FILE = 0x803
    IOCTL_KSH_MOVE_FILE = 0x804

    @classmethod
    def ctl_code(cls, function):
        file_device_unknown = 0x00000022
        method_buffered = 0
        file_access = 0x0002 | 0x0001
        return (
            (file_device_unknown << 16)
            | (file_access << 14)
            | (function << 2)
            | method_buffered
        )


def open_device():
    handle = win32file.CreateFile(
        r"\\.\KSH",
        win32file.GENERIC_READ | win32file.GENERIC_WRITE,
        0,
        None,
        win32file.OPEN_EXISTING,
        0,
        None,
    )
    if handle is None or handle == win32file.INVALID_HANDLE_VALUE:
        raise Exception("Failed to open device")
    return handle


def close_device(handle):
    win32file.CloseHandle(handle)


def device_io_ctl(code, input_buffer):
    handle = open_device()
    out_buffer = ctypes.c_ulong(0)
    bytes_returned = ctypes.c_ulong(0)

    status = ctypes.windll.kernel32.DeviceIoControl(
        handle.handle,
        CtlCode.ctl_code(code),
        ctypes.byref(input_buffer),
        ctypes.sizeof(input_buffer),
        ctypes.byref(out_buffer),
        ctypes.sizeof(out_buffer),
        ctypes.byref(bytes_returned),
        None,
    )

    close_device(handle)

    if not status or out_buffer.value != 0:
        error_code = ctypes.windll.kernel32.GetLastError()
        raise Exception(
            f"DeviceIoControl failed! Status {status},"
            f"Output {out_buffer.value}, Code {error_code},"
            f"Message: {win32api.FormatMessage(error_code)}"
        )


def format_dos_device(path):
    if not path.startswith("\\"):
        path = r"\DosDevices\{}".format(path)
    return path
