# gerber-renderer
3D PCB renderer for checking gerber files before manufacture

# Build Instructions

These instructions were tested on a fresh lubuntu 17.10 install. The the program doesn't seem to work with mesa software rendering.

__install pre-requsites__

sudo apt install git automake make gcc g++

__download and build gerbv (needed for the libraries)__

mkdir gerb3d

cd gerb3d

git clone git://git.geda-project.org/gerbv.git

cd gerbv

sudo apt install pkg-config libtool m4 libcairo2-dev gtk2.0

sh autogen.sh

./configure

make

sudo make install

sudo ldconfig

__download and build gerber-renderer__

cd ..

git clone https://github.com/tedyapo/gerber-renderer.git

cd gerber-renderer

sudo apt install mesa-common-dev freeglut3-dev libjson-c-dev

cd src

make

__test example__

cd ../examples

../src/gerb3d ddl_power_supply_release_v71.json
