# tinyqr
A QR generator that fits inside a QR code. Made in C/C++ for Win32

# credits
Thanks so much to the wonderfully simple [bjguillot/qr](https://github.com/bjguillot/qr) project for making a usable starting point for this project

# development process
After a few failed attempts to make anything worthwile in NASM, I got the Crinkler linker working in my environment. [Crinkler](https://github.com/runestubbe/Crinkler) is a compressing linker that outputs tiny executables, at the cost of high compile times and complexity.

## mission: 2.9 KB
From a working base weighing in at ~4.5 KB, I trimmed down the [bjguillot/qr](https://github.com/bjguillot/qr) library by locking the resulting QR to version 6, just before version info is required to read properly (which would have cost more space as code).

I was able to reduce the filesize to about ~3.2 KB through trimming unused logic and constants, but I hit a roadblock there. All the code left was used, and I'm not nearly smart enough to try to optimise the error correction algo implementation. So, I (assisted by copilot) rewrote the image renderer to output bitmaps instead of PNGs, because PNGs are complex.

That got me to 2.8 KB, just enough for a CLI to wrap the project up.

# how to use
reading binary QRs on windows is kinda difficult. so, there should be either a plain old exe build in the releases section, or further instructions below on how to do it properly.
