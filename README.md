# Datatrak LF Navigation Network mapping tools

Author: Phil Pemberton <philpem@gmail.com>

## Introduction

These tools take the almanac data from a Datatrak Locator (transmitter location table, slot-to-transmitter map, and active pattern list) and display it in the form of a graphical map.

The tools included are:

  * **Mapv**: Displays a map of all the transmitters, and the master-slave relationships.
  * **LOPMap**: Displays a map of all the transmitters, and the Lines of Position (LOPs) and Baselines.


## Build instructions

```bash
sudo apt install libtiff-dev libjpeg-turbo8-dev
make
```

Precompiling the CImg header may speed up compilation - implementing this is left as an exercise to the reader.


## Running the tools

  - Download the map data:
    * MapV requires [[http://www.shadedrelief.com/natural3/pages/textures.html|Natural Earth 3 "no clouds 16k"]] and [[http://shadedrelief.com/natural3/ne3_data/projection.zip|Natural Earth 3 projection data]]. This should be extracted into the `natural_earth_3` directory as `2_no_clouds_16k.jpg` and `2_no_clouds_16k.tfw` respectively.
    * LOPMap requires [[https://www.naturalearthdata.com/downloads/10m-raster-data/10m-cross-blend-hypso/|Natural Earth 1:10m Cross Blended Hypso with Shaded Relief and Water]] (the ZIP file includes the ESRI Worldfile projection data). This should be extracted into the `HYP_HR_SR_W` directory.
  - Build the tools (see above)
  - Run `lopmap` or `mapv` to generate the maps.

