Introduction
------------

Rocket-echo is a simple Linux kernel module that creates a simple loopback
character device with a parameterized ring-buffer size. It demonstrates the
creation, loading, and unloading of a Linux device driver.

Status
------

At present, this module is complete and serves as a good example of some key
concepts. The module compiles without warnings, and creates a device called
/dev/rocket which whill echo back anything that is written to it (unless the
buffer has already reached its limit).

Doxygen documentation has been completed, as well as a Makefile that can be
used to compile the kernel module.

