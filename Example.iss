;引入ist 编译器相对位置
#include "compiler:..\ist\script_ist.iss"

;安装包唯一ID
;32位生成注册表 HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\{#APP_GUID}_is1 64位没有WOW6432Node
;之下有用的键值
;QuietUninstallString 静默卸载字串 "{#INSTALL_DIR}\unins000.exe" /SILENT
;UninstallString 卸载字串 "{#INSTALL_DIR}\unins000.exe"
;InstallLocation 安装路径 "{#INSTALL_DIR}"
;DisplayName 显示名
;DisplayVersion 显示版本
;InstallDate 安装日期 yyyymmdd
#define APP_GUID "6C38DB0D-ABDE-40F6-8ADF-B7174E601C12"
;应用版本 并设置安装包版本
#define APP_VERSION "0.0.17.826"
;安装应用名
#define APP_NAME "My Program"
;应用英文名
#define APP_ENAME "My Program"
;默认的安装目录
;中途更改会继续以尾部目录名作为安装目录,如 C:\Program Files (x86)\APP -> D:\APP
#define INSTALL_DIR "D:\My Program"
;默认开始菜单分组
#define GROUP_NAME "My Program"
;发布者
#define APP_PUBLISHER "anoah"
;安装包输出目录
#define OUT_IDR "userdocs:Inno Setup Examples Output"
;EXE名称
#define APP_EXE "MyProg.exe"

[Setup]
AppId={{{#APP_GUID}}
;应用名
AppName={#APP_NAME}
;应用版本
AppVersion={#APP_VERSION}
;安装包版本
VersionInfoVersion={#APP_VERSION}
;默认开始菜单分组
DefaultGroupName={#GROUP_NAME}
;发布者
AppPublisher={#APP_PUBLISHER}
;卸载图标
UninstallDisplayIcon={app}\{#APP_EXE}
;压缩方式
Compression=lzma2
;固实压缩将启用,安装包较大时(超过100MB),设置为no
SolidCompression=yes
;设置管理员权限 poweruser, admin, or lowest
PrivilegesRequired=lowest
;生成安装包输出目录
OutputDir={#OUT_IDR}
;输出文件名
OutputBaseFilename={#APP_ENAME}_{#APP_VERSION}

[Files]
Source: compiler:Examples\MyProg.exe; DestDir: {app};AfterInstall:InstallFramework;Flags: ignoreversion replacesameversion
Source: compiler:Examples\MyProg.chm; DestDir: {app};Flags: ignoreversion replacesameversion
Source: compiler:Examples\Readme.txt; DestDir: {app};Flags: ignoreversion replacesameversion


[Icons]
Name: "{group}\{#APP_NAME}"; Filename: {app}\{#APP_EXE}


[UninstallDelete]
Name: {app};Type:filesandordirs

[Code]

{------------------安装------------------}

//安装初始化处理
function InitializeSetup(): Boolean;
var
isContinue:Boolean;//是否继续执行
begin
    isContinue:=true;
    //初始化皮肤
    if not ISTInitialize('{'+'{#APP_GUID}'+'}_is1','{#INSTALL_DIR}','{#APP_NAME}'+' {#APP_VERSION}') then begin
      isContinue:=false;
    end;
    Result:=isContinue;
end;

procedure InitializeWizard();
begin
  ISTSetWizard;//初始界面处理,隐藏WizardForm
end;


function ShouldSkipPage(PageID: Integer): Boolean;
begin
  Result := ISTInitialized;//不显示一些特定安装界面
end;

procedure CurPageChanged(CurPageID: Integer);
begin
   ISTSetPageChanged(CurPageID);//改变页面时隐藏界面窗口
end;

procedure CurInstallProgressChanged(CurProgress, MaxProgress: Integer);
begin
  //进度控制
  ISTSetProgressChanged(CurProgress,MaxProgress);
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
tmpRet:Integer;
begin
   tmpRet:=ISTSetStepChanged(CurStep);
   if -1=tmpRet then begin
      MsgBox('点击关闭',mbInformation, MB_OK);
   end else if 2=tmpRet then begin
      MsgBox('点击立即体验',mbInformation, MB_OK);
   end;
end;

{------------------卸载------------------}

function InitializeUninstall (): Boolean;
var
isContinue:Boolean;//是否继续执行
begin
    isContinue:=true;

    isContinue:=ISTInitializeU('{#APP_NAME}'+' {#APP_VERSION}');//初始化界面

    Result:=isContinue;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
tmpRet:Integer;
begin
  tmpRet:=ISTSetStepChangedU(CurUninstallStep);
  if -1=tmpRet then begin
      MsgBox('点击关闭',mbInformation, MB_OK);
  end else if 4=tmpRet then begin
      MsgBox('点击卸载完成',mbInformation, MB_OK);
  end;
end;

procedure DeinitializeUninstall();
begin
  SetDeinitializeU;
end;


{------------------其他------------------}

//耗时测试任务
procedure InstallFramework;
var
  //ResultCode: Integer;
  StatusText: String;
begin
    StatusText := WizardForm.StatusLabel.Caption;
    ISTSetTaskStatus('正在安装 .NET Framework',npbstMarquee);
    Sleep(3000);
    ISTSetTaskStatus('正在安装 VC++ Runtime',npbstMarquee);
    Sleep(3000);
    ISTSetTaskStatus(StatusText,npbstNormal);
  //一般使用的设置案例
  //StatusText := WizardForm.StatusLabel.Caption;
  //WizardForm.StatusLabel.Caption := '正在安装 .NET Framework 请稍等...';
  //WizardForm.ProgressGauge.Style := npbstMarquee;
  //try
  //  if not IsDotNetDetected('v4\Full',0) then
  //  begin
  //    if not Exec(ExpandConstant('{#BACKUP_DIR}\dotNetFx40_Full_x86_x64.exe'), '/q /norestart', '', SW_SHOW, ewWaitUntilTerminated, ResultCode) then
  //    begin
  //      MsgBox('.NET Framework4安装错误,Code:' + IntToStr(ResultCode) + ',请在稍后在安装目录,backup文件夹进行手动安装.',
  //        mbError, MB_OK);
  //    end;
  //  end;
  //finally
  //  WizardForm.StatusLabel.Caption := StatusText;
  //  WizardForm.ProgressGauge.Style := npbstNormal;
  //end;
end;