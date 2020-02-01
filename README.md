`icobuild` README
=================

I put together Windows icons with a tool that I wrote myself (platform
independent, source code included), along with some other Windows
tools like Paint.  However, as connectivity to the rest of the world
must tell, somebody else already wrote `ico-tools`, a program that can
do the same things my programs can do plus more.  The motivation for
writing `icobuild` was the same as for my other tool called `dlgedit`:
yeah MSVC++ 6.0 Enterprise includes a resource editor with icon
editing support, but that was expensive (before it became obsolete),
so why not write my own?

If you do not already have a program that can save bitmap files, you
can use ImageMagick to create the bitmap files.  Note that 32-bit
bitmap files may have certain non-standard peculiarities regarding the
format which they should be in before embedding into the icon file,
such as BI_RGB specified for biCompression.  To assist with this,
another tool called "bmptool" was provided.  When creating 4-bit
bitmaps, you should probably only use colors in the bitmap provided
called "vga-palette.bmp".  The problem with 32-bit bitmaps is
Microsoft's fault for not doing a good job programming the Windows
operating system.  ImageMagick does it right according to Microsoft's
documentation.

Also included is a file that I wrote earlier regarding how to make
Windows icons, intended to target Windows users.  However, it might
prove to be useful for cross-compiling users too.

If you are using an old version of the Windows Platform SDK resource
compiler, you will not be able to compile an icon containing a PNG
image into your binary.  This does not concern those who use "windres"
to compile the resources into their binary.
