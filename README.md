# liesel

Command-line core of BookThief

## Build Requirements

- Magick++ API (Packages: **graphicsmagick-libmagick-dev-compat** & **libmagick++-6-headers**)

- libfontconfig1 (Package: **libfontconfig1-dev**)

- PoDoFo 0.9.6 (Recommended to **compile from source**. You may also install the **libpodofo-dev** package instead, as this repository includes the **libpodofo.a** static library (the libpodofo-dev package does not include this library, though compiling from source will build it for you))

## Building

The makefile assumes that all static libraries **apart from libpodofo.a** (**libGraphicsMagick++.a**, and **libGraphicsMagick.a**) are located in **/usr/lib/**

**With** the aforementioned static libraries:

```
make
sudo make install
```

**Without** the aforementioned static libraries:

```
make nonstatic
sudo make install
```
