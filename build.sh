#!/usr/bin/bash

now="$(pwd)"

# if running on debian / ubuntu
if [ -f "$(which apt)" ] ; then
    sudo apt update
    sudo apt install mesa-common-dev build-essential git ninja-build cmake pkgconf libgstreamer1.0-dev libgstreamer-gl1.0-0 libgstreamer-plugins-base1.0-dev
fi

# if running on arch
if [ -f "$(which pacman)" ] ; then
    sudo pacman -Syu base-devel ninja cmake mesa gst-plugins-base-libs
fi

# install projectM
git clone https://github.com/projectM-visualizer/projectm
cd projectm
mkdir build && cd build
cmake -S ../ -B ./ -GNinja -DCMAKE_INSTALL_PREFIX=~/.local
ninja
ninja install
projectM4_DIR=$HOME/.local/lib/cmake/projectM4
cd ../..

# install gst-projectm
if [[ ! $(pwd) =~ "gst-projectm" ]] ; then
    git clone https://github.com/hashFactory/gst-projectm
    cd gst-projectm
fi
mkdir build && cd build
cmake -S ../ -B ./ -DprojectM4_DIR="$projectM4_DIR" -GNinja
ninja
mkdir -p $HOME/.local/share/gstreamer-1.0/plugins/
cp libgstprojectm.so $HOME/.local/share/gstreamer-1.0/plugins/
cd $now

# done!
echo "Done! now run with:"
echo "gst-launch-1.0 audiotestsrc ! queue ! audioconvert ! projectm ! \"video/x-raw,width=512,height=512,framerate=60/1\" ! videoconvert ! xvimagesink sync=false"
