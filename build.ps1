function build { & "C:\Program Files\NASM\nasm.exe" -f win32 .\sub.asm }
function link { .\Crinkler.exe /LARGEADDRESSAWARE:NO /PRIORITY:NORMAL /COMPMODE:VERYSLOW /TINYIMPORT /TINYHEADER /ENTRY:main kernel32.lib User32.lib .\headertest.obj }
function cbuild { cl.exe -c -D _NO_CRT_STDIO_INLINE -O1 -GS- -Gy -EHsc .\tinyqr.cpp }
function clink { .\Crinkler.exe /LARGEADDRESSAWARE:NO /PRIORITY:NORMAL /COMPMODE:VERYSLOW /TINYIMPORT /ENTRY:main kernel32.lib User32.lib vcruntime.lib msvcrt.lib .\tinyqr.obj }
