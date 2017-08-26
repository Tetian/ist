# ist
inno setup+duilib实现定制皮肤一键安装示例

## 示例
运行Example.exe可以看效果，其对应脚本Example.iss，使用可以参照这个脚本

## 文件结构说明
* script_ist.iss是ist.dll的一些操作封装
* script_utils.iss是一些工具方法
* ist.dll是生成的接口dll
* ist.zip是duilib皮肤资源
* src目录中生成ist.dll的vs项目，使用的是vs2015，生成时选择Release x86配置，duilib库需要clone引入

## 参考和引用库
* 思路参考：http://www.cnblogs.com/kissfancy/p/5736954.html
* 界面duilib库：https://github.com/duilib/duilib