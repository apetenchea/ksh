import hashlib
import ksh
import os
import psutil
import subprocess

MD5SUM = "d8a9e839260c00e65cc91be886638871"


def test_driver_test():
    ksh.test()


def test_driver_cp():
    from_file = "LICENSE"
    to_file = "hund"
    ksh.cp(
        os.path.abspath(".") + "\\" + from_file, os.path.abspath(".") + "\\" + to_file
    )
    assert os.path.exists(to_file)
    assert hashlib.md5(open(to_file, "rb").read()).hexdigest() == MD5SUM


def test_driver_mv():
    from_file = "hund"
    to_file = "katze"
    ksh.mv(
        os.path.abspath(".") + "\\" + from_file, os.path.abspath(".") + "\\" + to_file
    )
    assert os.path.exists(to_file)
    assert not os.path.exists(from_file)
    assert hashlib.md5(open(to_file, "rb").read()).hexdigest() == MD5SUM


def test_driver_rm():
    path = os.path.abspath(".") + "\\" + "katze"
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
