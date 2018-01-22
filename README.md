# gerber-renderer
3D PCB renderer for checking gerber files before manufacture

__Build Instructions__
# install pre-requsites
sudo apt install git automake make gcc g++

# download and build gerbv (needed for the libraries)
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

# download and install gerber-renderer
cd ..

git clone https://github.com/tedyapo/gerber-renderer.git

cd gerber-renderer

sudo apt install mesa-common-dev freeglut3-dev libjson-c-dev

cd src

make

# test example
cd ../examples

../src/gerb3d ddl_power_supply_release_v71.json
