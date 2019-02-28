# AQRouter prioritization service

## Description
AQRouter service expands functionality of AQtion Command Center, it allows to manage Internet activity initiated by different machines connected to this router. While AQtion Command Center prioritizes traffic on one PC where it is installed, the AQRouter service prioritizes all traffic in the subnet providing even better experience for online gaming, video streaming and communications with simultaneous network activity(e.g. downloading updates, file transfering).
 
Router connects to AQtion Command Center installed on one of the PCs in the local network and user can configure prioritization on the router in the special menu of the AQtion Command Center GUI.

## Requirements
For AQRouter service usage is required:
* OpenWRT router
* At least 1 AQtion Command Center client is running in LAN
* UI package (Optional)

## UI package
[LuCi package](https://github.com/Aquantia/luci-app-aqservice) may be build separatelly. That UI package provide next functionality:
* Enable/disable AQRouter service via WebUI
* Set management port for AQRouter service via WebUI
* Set password for management interface of AQRouter

## How to build package:
**```<openWRTdir>```** is openwrt source root directory 
1. Download **aqservice.diff** file from this repository.
2. Execute next commands:
   - cd **openWRTdir**
   - **./scripts/feeds update packages**
   - cd **openWRTdir/feeds/packages**
   - **git apply aqservice.diff**
   - cd **openWRTdir**
   - **./scripts/feeds update packages**
   - **./scripts/feeds install -a -p packages**
   - **make menuconfig**
3. Choose **Network -> Services -> aqservice** and press **select** button
4. Exit from menuconfig and save your configuration
5. Execute **make package/aqservice/compile**
6. Binary package with service may be found at **openWRTdir/bin/packages/proccessor arhitecture/packages/** as **aqservice.ipk** file