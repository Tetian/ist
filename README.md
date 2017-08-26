# ist
inno setup+duilib实现定制皮肤一键安装

## 示例
* 运行Example.exe可以看效果，其对应脚本Example.iss，使用可以参照这个脚本
* 卸载使用安装目录中或开始菜单组或控制面板中的"卸载"
![示例项目](https://git.oschina.net/uploads/images/2017/0826/185334_b5ec4ac8_1223664.png "示例项目")
![正式项目中的使用](https://git.oschina.net/uploads/images/2017/0826/185222_c7f3679a_1223664.png "正式项目中的使用")

## 文件结构说明
* script_ist.iss是ist.dll的一些操作封装，其中ist.dll和ist.zip和inno编译器路径存在相对关系：
```
\
\inno\Compil32.exe
...
\ist\ist.dll
\ist\ist.zip
\ist\script_ist.iss
```
* script_utils.iss是一些工具方法
* ist.dll是生成的接口dll
* ist.zip是duilib皮肤资源
* src目录中生成ist.dll的vs项目，使用的是vs2015，生成时选择Release x86配置，duilib库需要clone引入

## 参考和引用库
* 思路参考：http://www.cnblogs.com/kissfancy/p/5736954.html
* 界面duilib库：https://github.com/duilib/duilib

## TODO
* 还原异常弹出框，目前inno的异常弹出框不会出现