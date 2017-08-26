
;script_ist.iss
;Create 2017 Tetian
#ifndef ISS_SCRIPT_IST
#define ISS_SCRIPT_IST


[Preprocessor]

;引用utils
#ifndef ISS_SCRIPT_UTILS
#include "script_utils.iss"
#endif

;ist dll路径 这里根据inno编译器定位
#ifndef IST_DLL_PATH
#define IST_DLL_PATH "compiler:..\ist\ist.dll"
#endif

;ist dll文件名
#ifndef IST_DLL_NAME
#define IST_DLL_NAME "ist.dll"
#endif

;皮肤路径 这里更具inno编译器定位
#ifndef IST_SKIN_PATH
#define IST_SKIN_PATH "compiler:..\ist\ist.zip"
#endif

;皮肤文件名
#ifndef IST_SKIN_NAME
#define IST_SKIN_NAME "ist.zip"
#endif

;命令行 用于作为卸载标识
#ifndef CMD_IST
#define CMD_IST "/IST"
#endif

;用于作为静态卸载标识
#ifndef CMD_IST_SLIENT
#define CMD_IST_SLIENT "/IST_SLIENT"
#endif

;弹出框标题
#ifndef IST_MB_TITLE
#define IST_MB_TITLE "提示"
#endif

;卸载资源目录
#ifndef IST_UNINSTALL_DIR
#define IST_UNINSTALL_DIR "{app}\uninstall"
#endif

;进程检查方式
#ifndef IST_PORCESS_CHECK_TYPE
#define IST_PORCESS_CHECK_TYPE 1
#endif

;进程检查项
#ifndef IST_PROCESS_CHECK_EXE
#define IST_PROCESS_CHECK_EXE ""
#endif

;进程检查忽略项
#ifndef IST_PROCESS_CHECK_IGNORE
#define IST_PROCESS_CHECK_IGNORE ""
#endif

;安装dll接口函数标识
#ifndef IST_INSTALL_DLL_FLAGS
#define IST_INSTALL_DLL_FLAGS "stdcall setuponly delayload loadwithalteredsearchpath"
#endif

;卸载dll接口函数标识
#ifndef IST_UNINSTALL_DLL_FLAGS
#define IST_UNINSTALL_DLL_FLAGS "stdcall uninstallonly delayload loadwithalteredsearchpath"
#endif

[Setup]
;安装包默认安装路径
DefaultDirName={code:ISTDestDir}
;卸载exe放在特定目录
UninstallFilesDir={#IST_UNINSTALL_DIR}

[Files]
;dll
Source:{#IST_DLL_PATH}; flags: dontcopy nocompression noencryption solidbreak 
Source:{#IST_DLL_PATH}; DestDir: {#IST_UNINSTALL_DIR};Flags:ignoreversion replacesameversion
;皮肤
Source:{#IST_SKIN_PATH}; flags: dontcopy nocompression noencryption solidbreak 
Source:{#IST_SKIN_PATH}; DestDir: {#IST_UNINSTALL_DIR};Flags:ignoreversion replacesameversion

[Icons]
;创建卸载快捷方式在group和app
Name: "{group}\卸载"; Filename: "{uninstallexe}";Comment:"卸载程序";Parameters:"{#CMD_IST} /VERYSILENT /SUPPRESSMSGBOXES /NORESTART"
Name: "{app}\卸载"; Filename: "{uninstallexe}";Comment:"卸载程序";Parameters:"{#CMD_IST} /VERYSILENT /SUPPRESSMSGBOXES /NORESTART"


[Code]
{------------------安装定义------------------}
//安装皮肤初始化 使用DUILIB UILIB_ZIP初始皮肤
//is_type 1:安装 2:卸载
//skin_dir 皮肤路径
//skin_name 皮肤文件
//返回 true设置成功
function ISTInstallInit(is_type:Integer;skin_dir:String;skin_name:String):Boolean;
external 'ISTInstallInit@files:{#IST_DLL_NAME},{#IST_SKIN_NAME} {#IST_INSTALL_DLL_FLAGS}';

//设置初始界面安装路径
//install_dir 路径字串
procedure ISTSetInstallDir(install_dir:String);
external 'ISTSetInstallDir@files:{#IST_DLL_NAME},{#IST_SKIN_NAME} {#IST_INSTALL_DLL_FLAGS}';

//设置自定义数据
//data 自定义数据
procedure ISTSetCustomData(data:String);
external 'ISTSetCustomData@files:{#IST_DLL_NAME},{#IST_SKIN_NAME} {#IST_INSTALL_DLL_FLAGS}';

//显示安装操作界面
procedure ISTShowShell();
external 'ISTShowShell@files:{#IST_DLL_NAME},{#IST_SKIN_NAME} {#IST_INSTALL_DLL_FLAGS}';

//等待用户选择是否安装
//返回 安装时:-1点击了关闭按钮退出 1点击了"开始安装""
//卸载时: -1点击了关闭 3点击了"开始卸载"
function ISTWaitUserAction(): Integer;
external 'ISTWaitUserAction@files:{#IST_DLL_NAME},{#IST_SKIN_NAME} {#IST_INSTALL_DLL_FLAGS}';

//获取安装用户选择的路径
//ret 内容指针
//len 内容长度
procedure ISTGetInstallPath(ret:PChar;len:Integer);
external 'ISTGetInstallPath@files:{#IST_DLL_NAME},{#IST_SKIN_NAME} {#IST_INSTALL_DLL_FLAGS}';

//设置当前安装的进度
//now 当前安装的进度值
//max 安装进度最大值
//msg 安装信息
procedure ISTSetProgress(now:Integer;max:Integer;msg:String);
external 'ISTSetProgress@files:{#IST_DLL_NAME},{#IST_SKIN_NAME} {#IST_INSTALL_DLL_FLAGS}';

//设置安装文件信息
//msg 文件信息
procedure ISTSetFileName(msg:String);
external 'ISTSetFileName@files:{#IST_DLL_NAME},{#IST_SKIN_NAME} {#IST_INSTALL_DLL_FLAGS}';

//设置安装状态信息
//msg 状态信息
procedure ISTSetStatus(msg:String);
external 'ISTSetStatus@files:{#IST_DLL_NAME},{#IST_SKIN_NAME} {#IST_INSTALL_DLL_FLAGS}';

//等待用户点击完成
//返回 安装时:-1点击关闭按钮退出 2点击"立即体验"
//卸载时:-1点击关闭按钮 4卸载点击"卸载完毕"
function ISTWaitCompleted():Integer;
external 'ISTWaitCompleted@files:{#IST_DLL_NAME},{#IST_SKIN_NAME} {#IST_INSTALL_DLL_FLAGS}';

//显示模态提示框 未初始化皮肤时将使用系统MessageBox
//msg 消息
//caption 标题
//type_ 类型 0:MB_OK 其他:MB_OKCANCEL
//返回 IDOK(1) IDCANCEL(2)
function ISTShowMessage(msg:String;caption:String;type_:Integer): Integer;
external 'ISTShowMessage@files:{#IST_DLL_NAME},{#IST_SKIN_NAME} {#IST_INSTALL_DLL_FLAGS}';

//检查进程是否存在,并可设置尝试kill
//type 处理类型
//0:单进程处理
//1:进程名组处理
//2:文件夹处理,遍历文件夹内所有exe是否是正在运行的进程,会通过获取进程的完整路径进行路径比较
//如: D:\dir\a.exe D:\dir\b.exe D:\dir\c.exe D:\a.exe 运行的进程有a.exe b.exe设定路径为D:\dir 则只会对dir文件中的a.exe b.exe进行处理
//3:参数被包含进程处理,进程的参数是否包含了某些特定字符
//如: apache有多个进程 httpd.exe无参数和 参数包含-d及路径 需要关闭-d的进程
//4:参数不被包含进程处理,进程的参数不包含特定字符

//param1 参数1 根据type确定:
//type=0 单进程名"a.exe"
//type=1 进程名组"a.exe,b.exe"
//type=2 文件夹路径
//type=3或4 单进程名

//should_kill 进程运行是否尝试结束 1:尝试结束 0:不结束

//param2 参数2 根据type确定:
//type=0或1 保留项
//type=2 忽略目标 "ignore1.exe,ignore2.exe"
//type=3或4 被包含或不被包含命令行

//返回 0不存在或存在被kill掉 1存在 2存在但kill不掉:根据should_kill设置 1:返回0或2 0:返回0或1
function ISTProcessExisted(type_:Integer;param1:String;should_kill:Integer;param2:String): Integer;
external 'ISTProcessExisted@files:{#IST_DLL_NAME},{#IST_SKIN_NAME} {#IST_INSTALL_DLL_FLAGS}';

//常规进程检查:使用文件夹+进程组方式.
//文件夹适用情况:程序后期更新后可能会有新的运行进程exe放入安装目录 卸载时会自动排除掉卸载程序
//进程组适用情况:可能有其他不在安装目录中的关联运行进程;不受现在安装包控制的老版本运行进程 可和文件夹方式协同重复检查
//因进程名极可能重复,所以进程组参数设定极可能影响安装的运行进程即可
//type 1询问方式 2自动关闭 dir检查的文件夹;ignore忽略的exe组;exe_list进程组
procedure ISTSetProcessCheck(type_:Integer;const dir:String;const ignore:String;exe_list:String);
external 'ISTSetProcessCheck@files:{#IST_DLL_NAME},{#IST_SKIN_NAME} {#IST_INSTALL_DLL_FLAGS}';

{------------------卸载定义------------------}
function ISTInstallInitU(is_type:Integer;tmp_dir:String;skin:String):Boolean;
external 'ISTInstallInit@{#IST_UNINSTALL_DIR}\{#IST_DLL_NAME} {#IST_UNINSTALL_DLL_FLAGS}';

procedure ISTSetCustomDataU(data:String);
external 'ISTSetCustomData@{#IST_UNINSTALL_DIR}\{#IST_DLL_NAME} {#IST_UNINSTALL_DLL_FLAGS}';

procedure ISTShowShellU();
external 'ISTShowShell@{#IST_UNINSTALL_DIR}\{#IST_DLL_NAME} {#IST_UNINSTALL_DLL_FLAGS}';

function ISTWaitUserActionU(): Integer;
external 'ISTWaitUserAction@{#IST_UNINSTALL_DIR}\{#IST_DLL_NAME} {#IST_UNINSTALL_DLL_FLAGS}';

function ISTWaitCompletedU():Integer;
external 'ISTWaitCompleted@{#IST_UNINSTALL_DIR}\{#IST_DLL_NAME} {#IST_UNINSTALL_DLL_FLAGS}';

function ISTShowMessageU(msg:String;caption:String;type_:Integer): Integer;
external 'ISTShowMessage@{#IST_UNINSTALL_DIR}\{#IST_DLL_NAME} {#IST_UNINSTALL_DLL_FLAGS}';

function ISTProcessExistedU(type_:Integer;param1:String;should_kill:Integer;param2:String): Integer;
external 'ISTProcessExisted@{#IST_UNINSTALL_DIR}\{#IST_DLL_NAME} {#IST_UNINSTALL_DLL_FLAGS}';

procedure ISTSetProcessCheckU(type_:Integer;const dir:String;const ignore:String;exe_list:String);
external 'ISTSetProcessCheck@{#IST_UNINSTALL_DIR}\{#IST_DLL_NAME} {#IST_UNINSTALL_DLL_FLAGS}';

{------------------全局定义------------------}
//全局变量
var
//安装根目录
ISTDestFolder:string;
//是否使用IST安装
ISTUsed:Boolean;
//是否使用IST卸载
ISTUsedU:Boolean;
//安装是否完成初始化
ISTInitialized:Boolean;
//卸载是否完成初始化
ISTInitializedU:Boolean;
//APPGUID {}_is1
//用于获取原始安装信息
ISTAppGuid:String;

//返回安装路径
function ISTDestDir(SubDir:String):String;
begin
  if SubDir = '' then
    Result := ISTDestFolder
  else
    Result := ISTDestFolder + '\'+ SubDir;
end;

//返回卸载路径+参数
function ISTUninstallLink(tail:String):String;
var
TmpStr:String;
begin
    TmpStr:=' {#CMD_IST} /VERYSILENT /SUPPRESSMSGBOXES /NORESTART';
    if '' <> tail then begin
      TmpStr:=TmpStr+' '+tail;
    end;
    Result := ExpandConstant('"{uninstallexe}"' + TmpStr);
end;

//通用消息提示方法
//msg 消息
//itype_ ISS原始 TMsgBoxType = (mbInformation, mbConfirmation, mbError, mbCriticalError);
//MB_OK只有确定 MB_YESNO MB_OKCANCEL
function ISTMsgBox(msg:String;itype_:TMsgBoxType;button:Integer):Integer;
var
tmpReuslt:Integer;
begin
  if ISTInitialized then begin
   tmpReuslt:= ISTShowMessage(msg,'{#IST_MB_TITLE}',button);
  end else if ISTInitializedU then begin
   tmpReuslt:= ISTShowMessageU(msg,'{#IST_MB_TITLE}',button);
  end else begin
    tmpReuslt:= MsgBox(msg,itype_, button);
  end;
  Result:=tmpReuslt;
end;

{------------------安装------------------}

//释放dll
procedure ISTUnload();
begin
  UnloadDLL('{#IST_DLL_NAME}');
end;

//构造进程检查设定
procedure ISTSetProcessCheckBuild(appGuid:String;defInstallDir:String);
var
oriInstallDir,ignoreList,exeList:String;
begin
  oriInstallDir:=GetInstallDir(ISTAppGuid,defInstallDir);
  //安装时不用排除卸载exe
  ignoreList:='{#IST_PROCESS_CHECK_IGNORE}';
  exeList:='{#IST_PROCESS_CHECK_EXE}';
  ISTSetProcessCheck({#IST_PORCESS_CHECK_TYPE},oriInstallDir,ignoreList,exeList);
end;

//安装初始化
//appGuid:IS安装包注册表ID {--}_is1
//defInstallDir:默认安装路径
//customData:自定义数据 如界面上的其他文字
function ISTInitialize(appGuid:String;defInstallDir:String;customData:String):Boolean;
var
isContinue:Boolean;//是否继续执行
installDir:PChar;//选择的安装路径
TmpResult:Boolean;//返回结果 决定是否继续执行安装
begin
    //初始化全局变量
    ISTUsed:=true;
    ISTUsedU:=false;
    ISTInitialized:=false;
    ISTInitializedU:=false;
    ISTAppGuid:=appGuid;

    isContinue:=true;
    TmpResult:=true;

    //静态安装不加载IST CheckIsSilent是utils中过程
    isContinue:= not CheckIsSilent;

    if isContinue then begin
      Log('非静态安装,初始化皮肤');
      isContinue:=ISTInstallInit(1,ExpandConstant('{tmp}'),'{#IST_SKIN_NAME}');
      if not isContinue then begin
         ISTShowMessage('初始化错误','{#IST_MB_TITLE}',0);
         TmpResult:=false;
         Log('初始化错误错误,取消安装');
      end;
    end;

    //显示界面
    if isContinue then begin
      //设置原始安装路径 GetInstallDir是utils中函数
      ISTSetInstallDir(GetInstallDir(appGuid,defInstallDir));
      Log('设定自定义数据:'+customData);
      ISTSetCustomData(customData);
      //构造进程检查设定
      ISTSetProcessCheckBuild(appGuid,defInstallDir);
      //显示安装界面
      ISTShowShell;
    end;

    //等待用户操作
    if isContinue then begin
      //获取用户操作 -1为关闭
      isContinue:=-1<>ISTWaitUserAction;
      if not isContinue then begin
        //取消安装
        TmpResult:=false;
      end;
    end;

    //获取设置安装路径
    if isContinue then begin
      Setlength(installDir,1024);
      //获取安装路径 获取失败为空
      ISTGetInstallPath(installDir,1024);
      //设置获取的安装路径
      ISTDestFolder:=installDir;
      //为空停止安装
      isContinue:= ''<>ISTDestFolder;
      if not isContinue then begin
        ISTShowMessage('初始化错误:安装设定的路径获取错误','{#IST_MB_TITLE}',0);
        //取消安装
        TmpResult:=false;
      end;
    end;

    //设置初始化结果
    ISTInitialized:=isContinue;
    //初始化失败则释放DLL
    if not ISTInitialized then begin
      ISTUnload;
    end;

    Result:=TmpResult;
end;

//安装界面初始化设置
procedure ISTSetWizard();
begin
  if ISTInitialized then begin
    //隐藏主界面
    WizardForm.BorderStyle:=bsNone;
    WizardForm.ClientWidth := ScaleX(0);
    WizardForm.ClientHeight := ScaleY(0);
  end;
end;

//安装页面变化设置
procedure ISTSetPageChanged(CurPageID: Integer);
begin
   if ISTInitialized then begin
      //窗口尺寸变为0
      WizardForm.ClientWidth := ScaleX(0);
      WizardForm.ClientHeight := ScaleY(0);
      //欢迎自动点击下一步
      if CurPageID = wpWelcome then
        WizardForm.NextButton.OnClick(WizardForm);
      //安装时窗口隐藏
      if CurPageID >= wpInstalling then
        WizardForm.Visible:=false
      else
        WizardForm.Visible:=true;
    end;
end;

//进度设置
procedure ISTSetProgressChanged(CurProgress, MaxProgress: Integer);
begin
  if ISTInitialized then begin
    //设置进度条
    ISTSetProgress(CurProgress,MaxProgress,'');
    //设置安装文件信息
    ISTSetFileName(WizardForm.FilenameLabel.Caption);
    //设置状态信息
    ISTSetStatus(WizardForm.StatusLabel.Caption);
  end;
end;

//步骤控制
//返回 安装:-1:点击关闭按钮退出 2:安装点击"立即体验"
function ISTSetStepChanged(CurStep: TSetupStep):Integer;
//uninspath,uninsexe,newUninsFile,newUninsDatFile: string;
begin
  Result:=0;
  if CurStep=ssPostInstall then begin
    //设定新的卸载程序名 +修改了注册表 不需要设置卸载程序
    //卸载路径
    //uninspath:= ExtractFilePath(ExpandConstant('{uninstallexe}'));
    //卸载exe名
    //uninsexe:=ExtractFileName(ExpandConstant('{uninstallexe}'));
    //新卸载exe完整路径
    //newUninsFile:= uninspath + '{#IST_UNINSTALL_EXE}';
    //新卸载dat完整路径
    //newUninsDatFile:= uninspath + ISTChangeFileExt('{#IST_UNINSTALL_EXE}','dat');

    //删除原来的卸载文件
    //if FileExists(newUninsFile) then begin
    //  DeleteFile(newUninsFile);
    //end;
    //if FileExists(newUninsDatFile) then begin
    //  DeleteFile(newUninsDatFile);
    //end;

    //重命名新卸载文件
    //RenameFile(ExpandConstant('{uninstallexe}'), newUninsFile);
    //RenameFile(uninspath + ISTChangeFileExt(uninsexe,'dat'), newUninsDatFile);

    //设置安装注册信息
    //设置卸载字串
    SetInstallInfo(ISTAppGuid,'UninstallString',ISTUninstallLink(''));
    //设定静态卸载字串
    SetInstallInfo(ISTAppGuid,'QuietUninstallString',ISTUninstallLink('{#CMD_IST_SLIENT}'));
    //等待用户按结束
    if ISTInitialized then begin
      //安装等待结束
      Result:=ISTWaitCompleted;
    end;
  end;
 end;

//等待任务信息设置
procedure ISTSetTaskStatus(msg:String;progressStyle:TNewProgressBarStyle);
begin
  if ISTInitialized then begin
    //设置状态信息
    ISTSetStatus(msg);
  end else begin
    //设置INNO原生状态信息
    SetTaskStatus(msg,progressStyle);
  end;
end;



{------------------卸载------------------}
//释放dll
procedure ISTUnloadU();
begin
  UnloadDLL(ExpandConstant('{#IST_UNINSTALL_DIR}\{#IST_DLL_NAME}'));
end;

//构造忽略字串
function ISTBuildIgnoreExeListU(oriList:String):String;
var
TmpResult:String;
begin
  if ''=oriList then begin
    //'' -> {#IST_UNINSTALL_EXE}
    //TmpResult:='{#IST_UNINSTALL_EXE}';
    TmpResult:=ExtractFileName(ExpandConstant('{uninstallexe}'));
  end else begin
    //a.exe -> a.exe,{#IST_UNINSTALL_EXE} 或 a.exe,b.exe -> a.exe,b.exe,{#IST_UNINSTALL_EXE}
    //TmpResult:=oriList+','+'{#IST_UNINSTALL_EXE}';
    TmpResult:=oriList+','+ExtractFileName(ExpandConstant('{uninstallexe}'));
  end;
  Result:=TmpResult;
end;

//构造进程检查设定 排除掉对卸载程序的检测
procedure ISTSetProcessCheckBuildU();
begin
  ISTSetProcessCheckU({#IST_PORCESS_CHECK_TYPE},ExpandConstant('{app}'),ISTBuildIgnoreExeListU('{#IST_PROCESS_CHECK_IGNORE}'),'{#IST_PROCESS_CHECK_EXE}');
end;

//卸载初始
function ISTInitializeU(customData:String):Boolean;
var
isContinue:Boolean;//是否继续执行
TmpResult:Boolean;//返回结果 决定是否继续执行卸载
begin
    //初始化全局变量
    ISTUsed:=false;
    ISTUsedU:=true;
    ISTInitialized:=false;
    ISTInitializedU:=false;

    isContinue:=true;
    TmpResult:=true;

    if CheckContainCmd('{#CMD_IST_SLIENT}') and CheckIsSilent then begin
      //包含{#CMD_IST_SHELL} 和静态模式 则为'真'静态卸载
      isContinue:=false;
    end else if CheckContainCmd('{#CMD_IST}') and CheckIsSilent then begin
      //包含{#CMD_SHELL}和静态模式则为IST卸载
      isContinue:=true;
    end else begin
      //不包含静态模式的则为IS自带卸载界面
      isContinue:=false;
    end;

    //初始化皮肤
    if isContinue then begin
      isContinue:=ISTInstallInitU(2,ExpandConstant('{#IST_UNINSTALL_DIR}'),'{#IST_SKIN_NAME}');
      if not isContinue then begin
        ISTShowMessageU('初始化错误','{#IST_MB_TITLE}',0);
        //取消卸载
        TmpResult:=false;
      end;
    end;

    //显示卸载界面
    if isContinue then begin
      //设置自定义数据
      ISTSetCustomDataU(customData);
      //构造进程检查设定
      ISTSetProcessCheckBuildU;
      //卸载界面显示
      ISTShowShellU;
    end;

    //等待用户操作
    if isContinue then begin
      //获取用户操作 -1为关闭
      isContinue:=-1<>ISTWaitUserActionU;
      if not isContinue then begin
        //取消卸载
        TmpResult:=false;
      end;
    end;

    //设置卸载初始化结果
    ISTInitializedU:=isContinue;
    //初始化失败释放DLL
    if not ISTInitializedU then begin
      ISTUnloadU;
    end;

    Result:=TmpResult;
end;

//卸载步骤控制
//返回 安装:-1:点击关闭按钮退出 4卸载点击"卸载完毕"
function ISTSetStepChangedU(CurUninstallStep: TUninstallStep):Integer;
begin
  Result:=0;
  if CurUninstallStep=usPostUninstall then begin
    //等待用户按结束
    if ISTInitializedU then begin
      //卸载等待结束
      Result:=ISTWaitCompletedU;
    end;
  end;
end;

//退出卸载操作
procedure SetDeinitializeU();
begin
  if ISTInitializedU then begin
    //等待1秒,资源释放完毕并释放dll ISTWaitCompleted 后会设置释放标识 dll0.1秒检查"释放自己"
    //如果不释放则无法完全删除安装目录
    Sleep(1000);
    //ISTUnloadU; //自身释放
    DeleteFile(ExpandConstant('{#IST_UNINSTALL_DIR}\{#IST_DLL_NAME}'));
    DelTree(ExpandConstant('{app}'), True, True, True);
  end;
end;


[/Code]

#endif // ISS_SCRIPT_IST