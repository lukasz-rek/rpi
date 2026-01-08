# Info

This is a simple learning OS I wrote to get a better grasp on some of the operating system concepts. It started based on these guides, which I loosely followed taking relevant parts, modifying them slightly or translating to RPI 4 where needed.
 * [RPI 4 OS dev](https://github.com/sypstraw/rpi4-osdev) - graphical interface, mailbox calls
 * [s-matyukevich](https://github.com/s-matyukevich/raspberry-pi-os) - great guide with in-depth explanations! Helped me a lot with understanding user space, MMU and task scheduling.
 * [circle](https://github.com/rsta2/circle) - great reference for the EMMC2 driver, covers much more stuff so I had to cut out bits relevant to me :).

 I could continue to work more on this, but with OS's you have to know when to call it quits and I think my efforts now could be better spend on other projects. In the end, my system supports UART debug comms (used for key inputs as USB is a bit harder to implement), a task scheduler that supports sleeping tasks that can wait for ISR's and yield to other tasks in meantime. It has kernel/user space division with system calls and a graphical interface outputted on HDMI with a simple shell implementation. Then it also has a simple implementation of reading FAT32 files via EMMC2 from the sd-card. It also has a simple kernel heap and works in QEMU (though EMMC2 is not supported there yet, hence anything related to that doesn't work there).

If anyone also wants to write a simple OS up to a terminal, like I did, I hope this can be of some help!

# Some Pictures
Reading directories and ASCII art

<img src="docs/IMG_20260107_191541363.jpg" alt="Description" width="500">

Uptime, screen clear

<img src="docs/VID_20260107_191742214.gif" alt="Description" width="500">

Not great quality, I know, but couldn't capture it in QEMU.

# Possible improvements (not likely to happen tho)
* Implementation of FAT32 file writing. I know how to do it, but it would take some time to fully make & test.
* If above is done, a bootloader via UART would not be impossible and certainly help with testing on the board.
* The scheduler could be extended to pin task on cores for improved performance.
* Syscalls and libraries are not set up properly as I didn't know how to when writing it at first. With hindsight 20/20, I would rewrite them.
* With FAT32 working, some simple .elf files for user programs could be made and loaded.
* Likely more

