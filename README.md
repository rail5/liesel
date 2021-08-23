# liesel

Command-line core of BookThief

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

- PoDoFo 0.9.6 (Recommended to **compile from source**. This repository includes an optional pre-compiled **libpodofo.a** static library if you want it (the libpodofo-dev package does not include this library, though compiling from source will build it for you))

## Building

```
make
sudo make install
```

Static linking:

```
make static
sudo make install
```
