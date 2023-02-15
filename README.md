# liesel

Liesel turns ordinary PDFs into pamphlets/booklets which can be printed at home

This is **free software** (GNU GPL 3), and you are welcome to redistribute it under certain conditions.

![Demonstration](./liesel.gif)

## Features

*Note: Liesel is a* **command-line** *program. For a GUI front-end, see [BookThief](https://github.com/rail5/bookthief)*

Liesel will take any ordinary PDF (for example, an e-book) and combine/arrange the pages so that when the output PDF is printed, the entire stack of papers can just be folded in half to produce a booklet.

![Like this](https://cdn.zmescience.com/wp-content/uploads/2014/07/folding_paper.jpg)


All listed features are optional. A simple command like *liesel -i somebook.pdf -o booklet.pdf* will of course work just fine

Liesel also comes with a *manual page* to help guide you through some more advanced options. You can view it, after installing Liesel, by running **man liesel** in your terminal


 - Range input (i.e, process only the specified pages) (Example: *-r 1-5,7-10,3,20,100-90*)

 - Segmented output (produce multiple PDFs in segments of any given length (e.g, 40 pages per segment), to be more manageable when printed) (Example: *-s 40*)
 
 - Color-to-grayscale conversion (*-g*)

 - Automatic "landscape"/"long-edge" flipping (*-l*)
 
 - Specify arbitrary PPI / quality (Example: *-d 175*)
 
 - Transform/resize output PDF to print on any given paper size (Example: *-t 8.5x11*)

 - Export preview before running command (Example: *-e 5,6 -o preview.jpeg*)

 - Crop PDF pages (Example: *-C 10,20,30,40* crops 10% from the left, 20% from the right, 30% from the top, 40% from the bottom)

 - Convert to pure black-and-white (Not grayscale) (Example: *-k 50* changes every pixel under 50% brightness to black, every pixel over 50% to white)

 - Widen center margins (add blank space between left and right-hand sides) (Example: *-w 20*)

 - Auto-widen center margins (*progressively* add blank space between left and right-hand sides, wider toward the middle of the booklet) (Example: *-a* or *-a 50*)

 - Divide each page into two pages (for example, if the PDF is a scanned book with left and right-hand pages not separated) (*-D*)

The manpage and *liesel -h* will provide a comprehensive list of options and how to use them


## Installation

64-bit .deb packages are provided in the "Releases" section


On Ubuntu-based distros, Liesel can be easily installed via the BookThief PPA:

```
sudo add-apt-repository ppa:rail5/bookthief
sudo apt-get update
sudo apt-get install liesel
```

## Build Requirements

- Magick++ API (Packages: **graphicsmagick-libmagick-dev-compat** & **libmagick++-6-headers**)

- libfontconfig1 (Package: **libfontconfig1-dev**)

- Poppler (Package: **libpoppler-cpp-dev**)

## Building

```
make
sudo make install
```

## Supported Systems

Liesel is currently tested & verified to successfully build & run on:

 - GNU/Linux
 - MacOS
 - OpenBSD
 - Windows

If using OpenBSD, 'gmake' (GNU make) must be used rather than the default 'make'. Run 'gmake openbsd'

Windows binaries must be cross-compiled from GNU/Linux. Binaries for all other systems can be compiled from those systems themselves. See [autobuild](https://github.com/rail5/autobuild) for a script to automate the build process for GNU/Linux and Windows binaries
