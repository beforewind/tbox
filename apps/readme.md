

1. 生成visual studio(2017/2019)工程（Qtcreator打开CMakeLists.txt也能工作，项目不含QT组件）
-double click cmd.exe
:mkdir build
:cd build
:cmake ..
:打开vs2019工程，编译，F5调试运行cli_dll_test.exe，terminal输出网卡列表，选择正确的网卡序号作为参数输入，如：
：-n 3



