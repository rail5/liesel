# liesel

Liesel prepares ordinary PDFs to be home-printed in pamphlet/booklet form

This is **free software** (GNU GPL 3), and you are welcome to redistribute it under certain conditions.

## Features

*Note: Liesel is a* **command-line** *program. For a GUI front-end, see BookThief*

Liesel will take any ordinary PDF (for example, an e-book) and combine/arrange the pages so that when the output PDF is printed, the entire stack of papers can just be folded in half to produce a booklet.

This prints 2 pages per 1 side of a sheet of paper, and therefore 4 pages per sheet of paper.

All listed features are optional. A simple command like *liesel -i somebook.pdf -o booklet.pdf* will of course work just fine

 - Range input (i.e, process only pages ##-## from the input PDF)

 - Segmented output (produce multiple PDFs in segments of any given length (e.g, 40 pages per segment), to be more manageable when printed)
 
 - Color-to-grayscale conversion
 
 - Specify arbitrary PPI / quality
 
 - Transform/resize output PDF to print on any given paper size (e.g, *-t 8.5x11*)

The manpage and *liesel -h* will provide a comprehensive list of options and how to use them

## Some small notes

To print like this, of course, the pages have to go in a certain order. That order is:

last-page + first-page, second-page + second-to-last-page, third-to-last-page + third page, etc.

So, with 12 pages, it would be: 12,1,2,11,10,3,4,9,8,5,6,7

One consequence of this is that the number of pages needs to be even -- further, it should even be divisible by 4 (*4 pages per sheet of paper*).

In the case that a given input is not even or not divisible by 4, say for example that you have 14 pages, Liesel will add 2 blanks to the end. If these blanks were not added, your printer would use 4 sheets of paper and leave the backside of the final sheet blank -- this would place 2 blank pages in the middle of your booklet when you fold it over. By shifting things around, Liesel places them at the end of your booklet instead, where they make more sense. Of course, if you supply an odd number of pages, which when incremented by 1 yield an even number divisible by 4, Liesel will only add 1 blank. Likewise, if you supply an odd number which must be incremented by 3, it will add 3 blanks.

If you're concerned that you might end up paying a lot of money for ink -- *ink* is dirt-cheap, the cartridges are what cost (it's a big scam). You can refill your own cartridges (and there are also printers sold which are designed to be refilled, if you do have the money for one).

I've personally home-printed somewhere around a dozen novels using Liesel at the time of writing (2021-12-02) and use Liesel to print the news almost every day, at virtually no cost, by just refilling my printer's ink cartridges. With all the money that used to go to public libraries now being moved to (insert **current** war here), this program serves pretty well

## Installation

On Ubuntu-based distros, Liesel can be easily installed via the BookThief PPA:

```
sudo add-apt-repository ppa:rail5/bookthief
sudo apt-get update
sudo apt-get install liesel
```

## Build Requirements

- Magick++ API (Packages: **graphicsmagick-libmagick-dev-compat** & **libmagick++-6-headers**)

- libfontconfig1 (Package: **libfontconfig1-dev**)

- PoDoFo >= 0.9.6 (Package: **libpodofo-dev**) (Compile PoDoFo from source if you would like to do static linking, the libpodofo-dev package does not provide a static library)

## Building

```
make
sudo make install
```

Static linking (must compile PoDoFo from source to obtain a static library):

```
make static
sudo make install
```
