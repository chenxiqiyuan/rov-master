# [Underwater-Vehicle-NanoPi](https://github.com/chenxiqiyuan/Underwater-Vehicle-NanoPi.git)

## 1. 介绍

- 本项目用到了以下软件包：
  - [WiringNP](https://github.com/chenxiqiyuan/WiringNP.git) This is a GPIO access library for NanoPi. It is based on the WiringOP for Orange PI which is based on original WiringPi for Raspberry Pi.
  - [EasyLogger](https://github.com/armink/EasyLogger) 是一款超轻量级(ROM<1.6K, RAM<0.3K)、高性能的 C/C++ 日志库，非常适合对资源敏感的软件项目。

## 2. 使用
```shell
$ git clone https://github.com/zengwangfa/rovmaster
$ cd rovmaster
```

### 2.1 安装相关依赖
在第一次运行程序之前，务必执行:

```shell
$ chmod +x build.sh
$ ./build.sh

```
build.sh 中安装相关依赖库

### 2.2 编译与执行

```shell
$ make 
$ sudo ./rovmaster
```

### 2.3 注意事项
若执行时提示无法打开某设备，输入 `sudo npi-config` > `Advanced Options` 中使能相关设备：

```shell
$ sudo npi-config
```

若时区不对，也可以在 `npi-config` > `Localisation Options` > `Change Timezone` 中选择修改。
有时修改会后编译执行会出问题，未出现修改部分效果，此时 `make clean` 后再编译运行即可。

视频推流命令在 `video.sh` 脚本中，指定相关视频参数，在其中修改 **摄像头视频推流参数**即可。

## 3. 进度

- [ ] 模块进度
  - [ ] USB Camera
  - [x] ADC
    - [ ] 电压检测 (待实际测试)
    - [ ] 电流检测 (待实际测试)
  - [x] RGB LED
  - [ ] 探照灯
  - [ ] 按键
  - [x] 蜂鸣器
  - [x] 深度传感器
  - [x] 显示屏
  - [x] 九轴
  - [ ] PWM 
    - [x] 电压
    - [ ] 频率 (存在问题)
  - [x] CPU状况
    - [x] CPU温度
    - [x] CPU使用率

- [ ] 整体进度
  - [x] 传感器整合
  - [ ] 系统自检
  - [ ] 视频推流
  - [ ] NanoPi与上位机通信
  - [ ] 根据控制数据控制推进器