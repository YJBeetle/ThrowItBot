# TG_ThrowItBot

Throw It!

## Depend

```
sudo apt-get install g++ make binutils cmake
sudo apt-get install libssl-dev libboost-system-dev zlib1g-dev
sudo apt-get install libcairo2-dev libopencv-dev
```

```
brew install openssl zlib cmake boost@1.59
```

```
git clone https://github.com/reo7sp/tgbot-cpp
cd tgbot-cpp
cmake .
make -j4
sudo make install
```

ArtRobot: https://gitlab.com/project-null-plus-1/art_robot

## Build&&Test

    mkdir build && cd build && cmake ..
    make && TOKEN=<TOKEN> ./TG_ThrowItBot

## Run

    TOKEN=<TOKEN> ./TG_ThrowItBot