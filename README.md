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
