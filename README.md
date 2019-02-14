# AQService provides traffic prioritization using Aquantia Command Center

**```<openWRTdir>```** is openwrt source root directory 

How to build package:
1. Download **aqservice.diff** file from this repository.
1. Cd to **openWRTdir** and execute **./scripts/feeds update packages**
1. Cd to **openWRTdir/feeds/packages** and execute **git apply aqservice.diff**
1. Cd to **openWRTdir** and repeat **./scripts/feeds update packages**
1. Cd to **openWRTdir** and run **./scripts/feeds install -ap packages**
1. Run **make menuconfig** and choose **Network -> Services -> aqservice** and press **select** button
1. Exit from menuconfig and save your configuration
1. Run **make package/aqservice/compile**
1. Binary package may be found at **openWRTdir/bin/packages/proccessor arhitecture/packages/** as **aqservice.ipk** file