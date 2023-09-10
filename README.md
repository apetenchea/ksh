<div align="center">
  <a href="https://github.com/apetenchea/ksh">
    <img src="media/logo.png" alt="Logo">
  </a>

<h3 align="center">Kernel Shell</h3>
  <p>
    Because your Windows machine should listen to you!
  </p>
</div>

<details>
  <summary>Contents</summary>
  <ol>
    <li>
      <a href="#about">About</a>
      <ul>
        <li><a href="#motivation">Motivation</a></li>
        <li><a href="#features">Features</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#contributing">Contributing</a></li>
      <ul>
        <li><a href="#building">Building</a></li>
        <li><a href="#testing">Testing</a></li>
        <li><a href="#formatting">Formatting</a></li>
      </ul>
    <li><a href="#disclaimer">Disclaimer</a></li>
    <li><a href="#license">License</a></li>
  </ol>
</details>

## About

**ksh** is a kernel-mode Windows driver paired with a user-land Python script,
crafted to empower users with unparalleled control over their system.

### Motivation
At its core, the project is driven by two primary motivations:
1. **User Autonomy:** Modern operating systems, especially from Windows 10 onwards,
    have taken a more protective stance, restricting users from certain operations
    on their own machines. While this is often in the interest of security and stability,
    it sometimes feels like the system is impeding power users from performing advanced tasks.
    **ksh** seeks to break these chains, giving back users the freedom to dictate what they
    can and cannot do on their machines.
2. **Fun:** Diving deep into the internals of the OS and maneuvering data at the kernel level
    is a rewarding experience. Writing drivers is not just about the end utility but also about
    the thrill of exploration.

### Features
- **File Manipulation:** Basic operations like `rm`, `mv`, and `cp` are executed with the power
    and flexibility of kernel-level operations, bypassing many of the restrictions imposed by
    user-mode utilities.
- **Process Control:** Tools like `pkill` become more potent.

## Getting Started

### Prerequisites
- Windows 10 (x64)
- The driver is only signed with a test certificate, so you will need to enable test mode
    on your machine. This can be done by running the following command in an elevated command prompt:
    ```shell
    bcdedit.exe -set TESTSIGNING ON
    ```
    Note: This will require a reboot. **In case you're using BitLocker, be sure to have your recovery key at hand.**
- Python 3

### Installation
- Install Python requirements
    ```shell
    pip install -r requirements.txt
    ```
- Load the driver
    ```shell
    sc create ksh type=kernel binPath="path\to\ksh.sys"
    ```
- Start the driver
    ```shell
    sc start ksh
    ```
- Verify that the service is running
    ```shell
    sc query ksh
    ```
- Additionally, you can add the driver to the system's boot sequence. Make sure you've tested it first!
    ```shell
    sc config ksh start=boot
    ```
- When you're bored, you can stop the driver and unload it
    ```shell
    sc stop ksh
    sc config ksh start=demand
    sc delete ksh
    ```

## Usage
- Check the driver status
    ```shell
    python ksh.py test
    ```
- Move a file
    ```shell
    python ksh.py mv C:\Users\user\file.txt C:\Users\user\Documents\file.txt
    ```
- Copy a file
    ```shell
    python ksh.py cp C:\Users\user\file.txt C:\Users\user\Documents\file.txt
    ```
- Delete a file
    ```shell
    python ksh.py rm C:\Users\user\file.txt
    ```
- Kill a process
    ```shell
    python ksh.py pkill -n notepad.exe
    ```

## Contributing

Contributions are always welcome! Feel free to open an issue or submit a pull request.

### Building
- First, you need to install Visual Studio. I am using Visual Studio 2019 version 16.11.16. Other versions might work as well. 
- In order to build the driver component, you also need to install the Windows Driver Kit (WDK).  Here, I have
  used [WDK for Windows 10, version 2004](https://go.microsoft.com/fwlink/?linkid=2128854).
- Check out Microsoft's
  [other-wdk-downloads page](https://learn.microsoft.com/en-us/windows-hardware/drivers/other-wdk-downloads).
- Load the `driver.sln` solution in Visual Studio.
- Set the configuration to `Release` and the platform to `x64`.
- Before building the solution, run `bcdedit.exe -set TESTSIGNING ON` in an elevated command prompt. This allows the
  driver to be loaded with a real signature, and is going to be automatically signed with a test certificate during the
  build process. **In case you're using BitLocker, be sure to have your recovery key at hand before rebooting.**

### Testing
```shell
py.test test.py
```

### Formatting
- C: `clang-format -i -style=Microsoft *.h *.c`
- Python: `black *.py`

## Disclaimer

1. **Test Mode**  
    This driver requires the Windows "Test Mode" to be enabled in order to run. It's important to understand
    that operating in Test Mode can make your system vulnerable. In Test Mode, Windows will load any unsigned driver,
    which exposes your system to potential threats. Please be cautious and understand the risks before enabling Test Mode.
    After you're done using the driver, don't forget to disable Test Mode:
    ```shell
    bcdedit.exe -set TESTSIGNING OFF
    ```
2. **Potential System Damage**  
   Working with kernel-mode drivers carries inherent risks. Always ensure you know what the driver and any associated
   software are doing.
3. **Running experiments**  
   If you are experimenting or are unsure about the effects of this driver, it is highly recommended to run it in a
   controlled environment, such as a virtual machine.

## License
Distributed under the MIT License. See `LICENSE` for more information.