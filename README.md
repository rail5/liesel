# liesel

Liesel turns ordinary PDFs into pamphlets/booklets which can be printed at home.


> Rudy said, "You know something, Liesel, I was thinking. You’re not a thief at all. Stealing is what the army does. Taking your father, and mine." He kicked a stone and it clanged against a gate. He walked faster. "All those rich Nazis up there, on Grande Strasse, Gelb Strasse, Heide Strasse. How does it feel anyway?"
>
> "How does what feel?"
>
> "When you take one of those books?"
>
> At that moment, she chose to keep still. If he wanted an answer, he'd have to come back, and he did.
>
> "Well?" he asked, but again, it was Rudy who answered, before Liesel could even open her mouth.
>
> "It feels good, doesn't it? To steal something back."

– Markus Zusak's "The Book Thief"

## About

This is **free software** (GNU GPL 3), and you are welcome to redistribute it under certain conditions.

![Demonstration](./liesel.gif)

*Note: Liesel is a* **command-line** *program. For a GUI front-end, see [BookThief](https://github.com/rail5/bookthief)*

Liesel will take any ordinary PDF (for example, an e-book) and combine/arrange the pages so that when the output PDF is printed, the entire stack of papers can just be folded in half to produce a booklet.

![Like this](https://cdn.zmescience.com/wp-content/uploads/2014/07/folding_paper.jpg)

## Features

All listed features are optional. A simple command like *liesel -i somebook.pdf -o booklet.pdf* will of course work just fine

Liesel also comes with a *manual page* to help guide you through some more advanced options. You can view it, after installing Liesel, by running **man liesel** in your terminal

Here are just a few available options:

 - Range input (i.e, process only the specified pages) (Example: *-r 1-5,7-10,3,20,100-90*)

 - Segmented output (produce multiple PDFs in segments of any given length (e.g, 40 pages per segment), to be more manageable when printed) (Example: *-s 40*)
 
 - Convert to grayscale (*-g*)

 - Automatic "duplex" flipping (*-l*)
 
 - Specify PPI / quality (Example: *-d 175*)
 
 - Rescale output PDF to print on any arbitrary paper size (Example: *-t 8.5x11*)

 - Crop PDF pages (Example: *-C percentage-to-crop-from-the-left,right,top,bottom*, ie *-C 10,20,30,40*)

 - Convert to pure black-and-white (Not grayscale) (Example: *-k 50* changes every pixel under 50% brightness to black, every pixel over 50% to white)

 - Widen center margins (add blank space between left and right-hand sides) (Example: *-w 20*)

 - Divide each page into two pages (for example, if the PDF is a scanned book with left and right-hand pages not separated) (*-D*)

The manpage and *liesel -h* will provide a comprehensive list of all options and how to use them


## Installation

64-bit .deb packages and Windows binaries are provided in the "Releases" section


On Ubuntu-based distros, Liesel can be easily installed via the BookThief PPA:

```
sudo add-apt-repository ppa:rail5/bookthief
sudo apt-get update
sudo apt-get install liesel
```

## Build Requirements

- Magick++ API (Debian Packages: **graphicsmagick-libmagick-dev-compat** & **libmagick++-6-headers**)

- libfontconfig1 (Debian Package: **libfontconfig1-dev**)

- Poppler (Debian Package: **libpoppler-cpp-dev**)

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
