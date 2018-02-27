
## 首先下载openwrt的编译环境

下载openwrt的编译环境

``` 

git clone https://github.com/lixuande/openwrt-icbbox

```

探针的驱动层源码

```
cd openwrt-icbbox/package/
git clone https://github.com/lixuande/rt2860v2-detect

```

探针应用层源码

```
cd openwrt-icbbox/package/
git clone https://github.com/lixuande/rt2860v2-detect-user

```

  
## 其次进行配置和编译

openwrt配置和更新

```
cd openwrt-icbbox/
./scripts/feeds update -a
./scripts/feeds install -a
make menuconfig
```
make menuconfig会启动配置页面，在首页选上mt7620的无线驱动，同时要进入kernel的无线驱动下面去掉开源的驱动

```
Kernel modules-->Wireless Drivers-->kmod-rt2800-soc
Kernel modules-->Wireless Drivers-->kmod-rt2800-pci
Kernel modules-->Wireless Drivers-->kmod-rt2x00-lib

```

添加应用层获取探针数据的模块

```
Utilities-->apdetect
```

编译

```
cd openwrt-icbbox/
make -j4 V=99
```



这个博客中有详细说明http://blog.csdn.net/lixuande19871015/article/details/71601363#comments
注意选择好自己手头匹配的设备。
以上
