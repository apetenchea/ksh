<div style="text-align: center">
  <a href="https://github.com/apetenchea/ksh">
    <img src="media/logo.png" alt="Logo">
  </a>

<h3 align="center">Kernel Mode Shell</h3>
  <p>
    Because your Windows machine should listen to you!
  </p>
</div>

<details>
  <summary>Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About</a>
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
