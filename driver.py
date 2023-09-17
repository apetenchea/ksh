import ctypes
import getpass
import winreg
import win32security
from enum import IntEnum, Enum, auto

import win32api
import win32file
import struct


class CtlCode(IntEnum):
    IOCTL_KSH_PKILL = 0x800
    IOCTL_KSH_TEST = 0x801
    IOCTL_KSH_REMOVE_FILE = 0x802
    IOCTL_KSH_COPY_FILE = 0x803
    IOCTL_KSH_MOVE_FILE = 0x804
    IOCTL_KSH_REGEDIT = 0x805

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


class RegType(Enum):
    REG_SZ = auto()
    REG_DWORD = auto()
    REG_QWORD = auto()

    @classmethod
    def encode_buffer(cls, reg_type, data):
        if reg_type == cls.REG_SZ.name:
            return struct.pack("I", winreg.REG_SZ) + encode_string(str(data))
        elif reg_type == cls.REG_DWORD.name:
            return struct.pack("IIi", winreg.REG_DWORD, 4, int(data))
        elif reg_type == cls.REG_QWORD.name:
            return struct.pack("IIq", winreg.REG_QWORD, 8, int(data))
        else:
            raise Exception("Unknown reg type")


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


def device_io_ctl(code, input_buffer, input_size):
    handle = open_device()
    out_buffer = ctypes.c_ulong(0)
    bytes_returned = ctypes.c_ulong(0)

    status = ctypes.windll.kernel32.DeviceIoControl(
        handle.handle,
        CtlCode.ctl_code(code),
        input_buffer,
        input_size,
        ctypes.byref(out_buffer),
        ctypes.sizeof(out_buffer),
        ctypes.byref(bytes_returned),
        None,
    )

    close_device(handle)

    if not status or out_buffer.value != 0:
        error_code = ctypes.windll.kernel32.GetLastError()
        raise Exception(
            f"DeviceIoControl failed! Status {status}, "
            f"Output {out_buffer.value}, Code {error_code}, "
            f"Message: {win32api.FormatMessage(error_code)}"
        )


def format_dos_device(path):
    if not path.startswith("\\"):
        path = r"\DosDevices\{}".format(path)
    return path


def encode_string(s):
    wchar_s = s.encode("utf-16-le")
    size = len(wchar_s) + 2
    buffer = struct.pack("I", size) + wchar_s + b"\x00\x00"
    return buffer


def map_registry_path(p):
    if p.startswith(r"\Registry"):
        return p
    elif p.startswith("HKEY_LOCAL_MACHINE"):
        return p.replace("HKEY_LOCAL_MACHINE", r"\Registry\Machine")
    elif p.startswith("HKEY_USERS"):
        return p.replace("HKEY_USERS", r"\Registry\User")
    elif p.startswith("HKEY_CLASSES_ROOT"):
        return p.replace("HKEY_CLASSES_ROOT", r"\Registry\Machine\Software\Classes")
    elif p.startswith("HKEY_CURRENT_CONFIG"):
        return p.replace(
            "HKEY_CURRENT_CONFIG",
            r"\Registry\Machine\System\CurrentControlSet\Hardware Profiles\Current",
        )
    elif p.startswith("HKEY_CURRENT_USER"):
        username = getpass.getuser()
        sid, _, _ = win32security.LookupAccountName("", username)
        sid = win32security.ConvertSidToStringSid(sid)
        return p.replace("HKEY_CURRENT_USER", rf"\Registry\User\{sid}")
    else:
        raise Exception("Unknown registry path")
