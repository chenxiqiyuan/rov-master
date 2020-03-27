# [ROV-Master]

## 1. 介绍

- 本项目用到了以下软件包：
  - [WiringNP](https://github.com/friendlyarm/WiringNP) This is a GPIO access library for NanoPi. It is based on the WiringOP for Orange PI which is based on original WiringPi for Raspberry Pi.
  - [EasyLogger](https://github.com/armink/EasyLogger) 是一款超轻量级(ROM<1.6K, RAM<0.3K)、高性能的 C/C++ 日志库，非常适合对资源敏感的软件项目。

## 2. 使用
```shell
$ git clone https://github.com/zengwangfa/rovmaster
$ cd rovmaster
```

### 2.1 安装相关依赖
在第一次运行程序之前，务必执行(`build.sh` 自动化安装依赖库)):

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
- 驱动模块
	- [x] ADS1118 (ADC)
		- [ ] 电压检测 (待实际测试)
		- [ ] 电流检测 (待实际测试)
	- [ ] PCA9685 (PWM) 
	- [x] 深度传感器
		- [x] SPL1301 (待实际测试)
		- [x] MS5837  (待实际测试)
	- [x] CPU状况
		- [x] CPU温度
		- [x] CPU使用率
	- [ ] IO设备
		- [x] RGB
		- [ ] 按键
		- [x] 蜂鸣器
	- [ ] PWM设备
		- [ ] 云台
		- [ ] 探照灯
		- [ ] 机械臂
	- [ ] OLED
	- [x] 九轴
	- [ ] 音频MIC

- 功能模块
	- [x] 视频推流
	- [ ] 数据通信
		- [x] 下行-控制数据
		- [x] 上行-ROV状态数据
	- [ ] 系统自检
	- [ ] 日志记录
	- [ ] 传感器融合

- 控制模块
	- [ ] 手柄控制
	- [ ] 定深控制
	- [ ] 定向控制
	- [ ] 变焦摄像头控制


