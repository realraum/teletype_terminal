## How to checkout and build?

```bash
git clone --recurse-submodules git@github.com:realraum/teletype_terminal.git
cd teletype_terminal/teletype_esp32
esp-idf/install.sh
. esp-idf/export.sh

# build the project
idf.py build

# flash the project
idf.py -p /dev/ttyUSB0 -b 921600 flash monitor -b 115200

# just update app partition
idf.py -p /dev/ttyUSB0 -b 921600 app-flash monitor -b 115200
```
