import driver
import hashlib
import ksh
import os
import psutil
import pytest
import subprocess
import winreg


MD5SUM = "d8a9e839260c00e65cc91be886638871"
TEST_REG_KEY = r"Software\TestKey"
TEST_REG_VALUE = "TestValue"


def get_file_path(file):
    return os.path.abspath(".") + "\\" + file


@pytest.fixture(scope="session", autouse=True)
def teardown_all():
    yield

    # Cleanup files
    if os.path.exists("hund"):
        os.remove("hund")

    # Cleanup registry
    try:
        with winreg.OpenKey(
            winreg.HKEY_CURRENT_USER, TEST_REG_KEY, 0, winreg.KEY_ALL_ACCESS
        ) as key:
            winreg.DeleteKey(key, TEST_REG_VALUE)
        winreg.DeleteKey(winreg.HKEY_CURRENT_USER, TEST_REG_KEY)
    except (FileNotFoundError, PermissionError):
        pass


def test_driver_test():
    ksh.test()


def test_driver_cp():
    from_file = "LICENSE"
    to_file = "hund"
    ksh.cp(get_file_path(from_file), get_file_path(to_file))
    assert os.path.exists(to_file)
    assert hashlib.md5(open(to_file, "rb").read()).hexdigest() == MD5SUM


def test_driver_mv():
    from_file = "hund"
    to_file = "katze"
    ksh.mv(get_file_path(from_file), get_file_path(to_file))
    assert os.path.exists(to_file)
    assert not os.path.exists(from_file)
    assert hashlib.md5(open(to_file, "rb").read()).hexdigest() == MD5SUM


def test_driver_rm():
    path = get_file_path("katze")
    ksh.rm(path)
    assert not os.path.exists(path)


def test_driver_pkill():
    notepad = "notepad.exe"
    process = subprocess.Popen([notepad])
    pid = process.pid
    ksh.pkill(pid)
    for proc in psutil.process_iter():
        if proc.name() == notepad:
            assert pid != proc.pid


def test_driver_regedit():
    reg_path = rf"HKEY_CURRENT_USER\{TEST_REG_KEY}"
    for k, v in (
        (driver.RegType.REG_DWORD, 1234),
        (driver.RegType.REG_QWORD, 12345443232),
        (driver.RegType.REG_SZ, "hund"),
    ):
        ksh.regedit(k.name, v, reg_path, TEST_REG_VALUE)
        with winreg.OpenKey(
            winreg.HKEY_CURRENT_USER, TEST_REG_KEY, 0, winreg.KEY_READ
        ) as key:
            res = winreg.QueryValueEx(key, TEST_REG_VALUE)
            assert res[0] == v
