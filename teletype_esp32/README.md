## How to checkout and build?

```
git clone --recursive git@github.com:realraum/teletype_terminal.git
cd teletype_terminal/teletype_esp32
git submodule update --init --recursive
esp-idf/install.sh
. esp-idf/export.sh
idf.py build
idf.py -p /dev/ttyUSB0 -b 921600 flash monitor -b 115200
idf.py -p /dev/ttyUSB0 -b 921600 app-flash monitor -b 115200
```
