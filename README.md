# liesel

Command-line core of BookThief

## Build Requirements

- Magick++ API (Packages: **libmagick++-6.q16-8** & **libmagick++-6-headers**)

- PoDoFo 0.9.6 (Recommended to **compile from source**. If you install the **libpodofo-dev** package instead, run **make nonstatic** as that package does not include static libraries)

## Building

The makefile assumes that all static libraries (**libGraphicsMagick++.a**, **libGraphicsMagick.a** and **libpodofo.a**) are located in **/usr/lib/**

**With** the aforementioned static libraries:

```
make
```

**Without** the aforementioned static libraries:

```
make nonstatic
```
