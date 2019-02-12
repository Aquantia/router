# router
AQService  provides traffic prioritization using Aquantia Command Center

How to build package:
* Checkout package from https://github.com/Aquantia/router.git and find file aqservice.diff
* Cd to openwrt src root directory
* Run ./scripts/feeds update packages
* Cd to feeds/packages and run git apply aqservice.diff
* Run again ./scripts/feeds update packages
* Cd to openwrt src root directory and run ./scripts/feeds install -ap packages
* Run make menuconfig and shoose Network -> Services -> aqservice and press select 
* Exit from menuconfig and save your configuration
* Run make package/aqservice/compile
* Cd to bin/packages/<Processor arhitecture>/packages/ and find aqservice.<version><arhitecture>.ipk