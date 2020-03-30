#!/bin/sh 
#这个脚本的作用是自动安装相关的软件、安装相关的库
#需要root权限来运行这个脚本


echo "=====================安装WiringNP=========================="
cd ./lib/WiringNP/
chmod 755 build
./build


echo "=================安装mjpg-streamer及相关依赖================="
sudo apt-get update
sudo apt-get install -y cmake libjpeg8-dev
cd ../mjpg-streamer-experimental/
make
sudo make install

echo "=======================安装easylogger======================"
# easylogger 安装
cd ../easylogger
make
sudo cp libeasylogger.so ..
sudo cp libeasylogger.so /usr/lib/

cd ..

exit 0
