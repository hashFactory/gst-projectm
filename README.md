
## gst-projectM - A projectM plugin for Gstreamer

This is still a work in progress but should build on most common linux system setups.

### Build automatically

Supported platforms:
- Ubuntu / Debian
- Arch linux / Manjaro

```bash
chmod +x ./build.sh
./build.sh
```

### Build manually

#### Prerequisites:
- [ProjectM](https://github.com/projectM-visualizer/projectm)
- [GStreamer](https://gitlab.freedesktop.org/gstreamer/gstreamer)

#### Build libprojectm
```bash
git clone https://github.com/projectM-visualizer/projectm
cd projectm
mkdir build && cd build
cmake -S ../ -B ./ -GNinja -DCMAKE_INSTALL_PREFIX=~/.local
ninja
ninja install
export projectM4_DIR=$HOME/.local/lib/cmake/projectM4
cd ../..
```

#### Build gst-projectM
```bash
git clone https://github.com/hashFactory/gst-projectm
cd gst-projectm
mkdir build && cd build
cmake -S ../ -B ./ -DprojectM4_DIR="$projectM4_DIR" -GNinja
ninja
mkdir -p $HOME/.local/share/gstreamer-1.0/plugins/
cp libgstprojectm.so $HOME/.local/share/gstreamer-1.0/plugins/
```

### How To Use
```bash
# if libgstprojectm.so is not in its default path
export GST_PLUGIN_PATH="[ location of folder containing libgstprojectm.so ]"

# Record desktop audio and display it in a 1920x1080 window
gst-launch-1.0 pulsesrc ! audioconvert ! queue ! \
    projectm preset="presets/cool_dots.milk" ! "video/x-raw,width=1920,height=1080,framerate=60/1,format=BGRA" ! \
    queue ! videoconvert ! autovideosink sync=false

# Read audio from a file and hardware encode the projectM video output with the audio stream
gst-launch-1.0 filesrc location="another.wav" ! wavparse ! tee name=t \
    ! queue ! matroskamux name=mux ! filesink location="recording.mkv" \
    t. ! queue ! audioconvert ! projectm preset="presets/cool_dots.milk" \
    ! "video/x-raw,width=1080,height=2440,framerate=60/1,format=BGRA" ! queue \
    ! vaapipostproc ! vaapih264enc ! video/x-h264,profile=main ! h264parse ! mux.
```
