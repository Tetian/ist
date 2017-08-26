;script_utils.iss
;Create 2015 Tetian
;IsDotNetDetected 检查.Net是否安装
;IsDotNet4 .Net4.0是否安装
;SetInstallInfo 设置安装信息 尝试设置HKCU和HKLM
;GetInstallInfo 获取安装信息 先从HKCU获取,失败则从HKLM获取
;BeforeUninstall_Inno 对Inno结构的安装程序进行卸载
;BeforeUninstall_MSI  对MSI结构的安装程序进行卸载
;SetEnv 设置环境变量 需要管理员权限
;CheckContainCmd 检查安装包启动参数是否包含某个特定值
;CheckIsSilent 检查是否包含静默安装参数
;SetFirewallException 设置程序防火墙例外
;RemoveFirewallException 移除程序防火墙例外
;SetFirewallPortException 设置协议端口防火墙例外
;RemoveFirewallPortException 移除协议防火墙例外
;VCVersionInstalled 检查对应版本VC++是否安装
;SetTaskStatus 设置任务信息,在安装过程中设置
;CompareVersion 比较版本
;LastCharPos 反向查询字符位置
;ChangeFileExt 替换文件扩展名

#ifndef ISS_SCRIPT_UTILS
#define ISS_SCRIPT_UTILS

[Code]
{判断.NET Framework 是否安装}
//version -- required .NET Framework version:
//    'v1.1'          .NET Framework 1.1
//    'v2.0'          .NET Framework 2.0
//    'v3.0'          .NET Framework 3.0
//    'v3.5'          .NET Framework 3.5
//    'v4\Client'     .NET Framework 4.0 Client Profile
//    'v4\Full'       .NET Framework 4.0 Full Installation
//
//  service -- required service pack level:
//    0               No service packs required
//    1, 2, etc.      Service pack 1, 2, etc. required
function IsDotNetDetected(version: String; service: Cardinal): Boolean;
var
    key: String;
    install, count: Cardinal;
    success: Boolean;
begin
    if (version = 'v1.1') then version := version + '.4322'
    else if (version = 'v2.0') then version := version + '.50727';
    key := 'SOFTWARE\Microsoft\NET Framework Setup\NDP\' + Version;
    
    // .NET 3.0 uses value InstallSuccess in subkey Setup
    if Pos('v3.0', version) = 1 then success := RegQueryDWordValue(HKLM, key + '\Setup', 'InstallSuccess', install)
    else success := RegQueryDWordValue(HKLM, key, 'Install', install);
    
    // .NET 4.0 uses value Servicing instead of SP
    if Pos('v4', version) = 1 then success := success and RegQueryDWordValue(HKLM, key, 'Servicing', count)
    else success := success and RegQueryDWordValue(HKLM, key, 'SP', count);
    
    Result := success and (install = 1) and (count >= service);
end;

{判断.net4.0 v4\Full版本是否安装}
function IsDotNet4(): Boolean;
begin
      Result:=IsDotNetDetected('v4\Full',0);
end;

{判断.net2.0版本是否安装}
function IsDotNet2(): Boolean;
begin
      Result:=IsDotNetDetected('v2.0',0);
end;


{设置安装信息}
function SetInstallInfo(appName:String;regKey:String;val:String):Boolean;
var
ResultStr: String;
RegSoftwareKey:String;
TmpResult:Boolean;
begin
	TmpResult:=false;
	RegSoftwareKey:='SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\'+appName;
	{PrivilegesRequired=lowest}
	if RegQueryStringValue(HKCU, RegSoftwareKey, regKey, ResultStr) then
    begin
		ResultStr := RemoveQuotes(ResultStr);
		TmpResult:=RegWriteStringValue(HKCU, RegSoftwareKey, regKey, val);
  end;
	{PrivilegesRequired=admin}
  if RegQueryStringValue(HKLM, RegSoftwareKey, regKey, ResultStr) then
	begin
		ResultStr := RemoveQuotes(ResultStr);
		TmpResult:=TmpResult or RegWriteStringValue(HKLM, RegSoftwareKey, regKey, val);
	end;
  Result:=TmpResult;
end;

{获取安装信息}
function GetInstallInfo(appName:String;regKey:String;defValue:String):String;
var
ResultStr: String;
begin
	ResultStr:=defValue;
	{PrivilegesRequired=lowest}
	if RegQueryStringValue(HKCU, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\'+appName, regKey, ResultStr) then
    begin
		ResultStr := RemoveQuotes(ResultStr);
	{PrivilegesRequired=admin}
    end else if RegQueryStringValue(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\'+appName, regKey, ResultStr) then
	begin
		ResultStr := RemoveQuotes(ResultStr);
	end;
    Result := ResultStr;
end;

{获取安装路径,获取失败返回默认值}
function GetInstallDir(appName:String;defDir:String):String;
begin
    Result := GetInstallInfo(appName,'InstallLocation',defDir);
end;

{安装前卸载Inno}
{appName是GUID,msgInfo:提示消息,为空不提示,silent:是否静默执行}
function BeforeUninstall_Inno(appName:String;msgInfo:String;silent:boolean): boolean;
var
	ResultStr: String;
	Param:String;
	ResultCode: Integer;
	IsContinue:Boolean;
begin
	IsContinue:=true;
	{获取注册表信息}
	ResultStr:=GetInstallInfo(appName,'UninstallString','');
	if '' = ResultStr then begin
		IsContinue:=false;
	end;
	{提示框验证}
	if IsContinue then begin
		if ''<>msgInfo then begin
			if MsgBox(msgInfo, mbConfirmation, MB_YESNO) <> IDYES then begin
				IsContinue:=false;
			end;
		end;
	end;
	{执行卸载}
	if IsContinue then begin
		Param:='';
		if silent then begin
			Param:='/silent';
		end;
		if not ShellExec('',ResultStr, Param, '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then begin
			MsgBox('卸载:'+appName+'发生错误,Code:' + IntToStr(ResultCode) +'.', mbError, MB_OK);
			IsContinue:=false;
		end;
	end;
	
	Result:=IsContinue; 
end;

{安装前卸载MSI}
{pid:MSI ProductID,msgInfo:提示消息,为空不提示,silent:是否静默执行}
function BeforeUninstall_MSI(pid:String;msgInfo:String;silent:boolean): boolean;
var
	ResultStr:String;
	Param:String;
	ResultCode:Integer;
	TmpResult,IsContinue:boolean;
begin
	IsContinue:=true;
  TmpResult:=true;
	{获取注册表信息}
	ResultStr:=GetInstallInfo(pid,'UninstallString','');
	if ''=ResultStr then begin
		IsContinue:=false;
	end;
	{验证提示框}
	if IsContinue then begin
		if ''<>msgInfo then begin
			if MsgBox(msgInfo, mbConfirmation, MB_YESNO) <> IDYES then begin
				IsContinue:=false;
        TmpResult:=false;
			end;
		end;
	end;
	{执行卸载}
	if IsContinue then begin
		Param:=' /uninstall '+pid;
            if silent then begin
                Param:=Param+' /qb';
            end;
        if not ShellExec('','MsiExec.exe', Param,'', SW_SHOW, ewWaitUntilTerminated, ResultCode) then begin
			MsgBox('卸载:'+pid+'发生错误,Code:' + IntToStr(ResultCode) +'.', mbError, MB_OK);
			IsContinue:=false;
		end;
	end;
	
	Result:=TmpResult;
end;



{设置环境变量}
{aIsInstall增加还是删除，true增加环境变量，false删除环境变量}
procedure SetEnv(aEnvName, aEnvValue: string; aIsInstall: Boolean);
var
sOrgValue: string;
x,len: integer;
begin
    {得到以前的值}
    RegQueryStringValue(HKLM, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', aEnvName, sOrgValue);
    sOrgValue := Trim(sOrgValue);
    begin
      x := pos( Uppercase(aEnvValue),Uppercase(sOrgValue));
      len := length(aEnvValue);
	  {是安装还是反安装}
      if aIsInstall then
      begin
          if length(sOrgValue)>0 then aEnvValue := ';'+ aEnvValue;
          if x = 0 then Insert(aEnvValue,sOrgValue,length(sOrgValue) +1);
      end
      else
      begin
         if x>0 then Delete(sOrgValue,x,len);
         if length(sOrgValue)=0 then
         begin
           RegDeleteValue(HKLM, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',aEnvName);
           exit;
         end;
      end;
      StringChange(sOrgValue,';;',';');
      RegWriteStringValue(HKLM, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', aEnvName, sOrgValue)
    end;
end;

{检查是否包含指定命令}
function CheckContainCmd(cmd:string):Boolean;
begin
    Result:=Pos(Lowercase(cmd), Lowercase(GetCmdTail)) > 0;
end;

{检查是否是静态安装模式}
function CheckIsSilent():Boolean;
begin
    Result:= CheckContainCmd('/SILENT') or CheckContainCmd('/VERYSILENT');
end;

{防火墙操作相关}
const
  NET_FW_SCOPE_ALL = 0;
  NET_FW_IP_VERSION_ANY = 2;
{增加应用防火墙例外}
procedure SetFirewallException(AppName,FileName:string);
var
  FirewallObject: Variant;
  FirewallManager: Variant;
  FirewallProfile: Variant;
begin
  try
    FirewallObject := CreateOleObject('HNetCfg.FwAuthorizedApplication');
    FirewallObject.ProcessImageFileName := FileName;
    FirewallObject.Name := AppName;
    FirewallObject.Scope := NET_FW_SCOPE_ALL;
    FirewallObject.IpVersion := NET_FW_IP_VERSION_ANY;
    FirewallObject.Enabled := True;
    FirewallManager := CreateOleObject('HNetCfg.FwMgr');
    FirewallProfile := FirewallManager.LocalPolicy.CurrentProfile;
    FirewallProfile.AuthorizedApplications.Add(FirewallObject);
  except
  end;
end;

{移除应用防火墙例外}
procedure RemoveFirewallException( FileName:string );
var
  FirewallManager: Variant;
  FirewallProfile: Variant;
begin
  try
    FirewallManager := CreateOleObject('HNetCfg.FwMgr');
    FirewallProfile := FirewallManager.LocalPolicy.CurrentProfile;
    FireWallProfile.AuthorizedApplications.Remove(FileName);
  except
  end;
end;

//*****示例
//procedure CurStepChanged(CurStep: TSetupStep);
//begin
//  if CurStep=ssPostInstall then
//     SetFirewallException('My Server', ExpandConstant('{app}\')+'TCPServer.exe');
//end;
//
//procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
//begin
//  if CurUninstallStep=usPostUninstall then
//     RemoveFirewallException(ExpandConstant('{app}\')+'TCPServer.exe');
//end;


const	
  NET_FW_PROTOCOL_TCP = 6;
  NET_FW_PROTOCOL_UDP = 17;
{设置端口协议防火墙例外}
procedure SetFirewallPortException(AppName: string; Protocol, Port: integer);
var
  FirewallObject: Variant;
  FirewallManager: Variant;
  FirewallProfile: Variant;
begin
  try
    FirewallObject := CreateOleObject('HNetCfg.FwOpenPort');
    FirewallObject.Name := AppName;
    FirewallObject.Scope := NET_FW_SCOPE_ALL;
    FirewallObject.IpVersion := NET_FW_IP_VERSION_ANY;
    FirewallObject.Protocol := Protocol;
    FirewallObject.Port := Port;
    FirewallObject.Enabled := True;
    FirewallManager := CreateOleObject('HNetCfg.FwMgr');
    FirewallProfile := FirewallManager.LocalPolicy.CurrentProfile;
    FirewallProfile.GloballyOpenPorts.Add(FirewallObject);
  except
  end;
end;    
{移除端口协议防火墙例外}
procedure RemoveFirewallPortException(Protocol, Port: integer);
var
  FirewallManager: Variant;
  FirewallProfile: Variant;
begin
  try
    FirewallManager := CreateOleObject('HNetCfg.FwMgr');
    FirewallProfile := FirewallManager.LocalPolicy.CurrentProfile;
    FireWallProfile.GloballyOpenPorts.Remove(Port, Protocol);
  except
  end;
end;




{VC++库检验相关}
type
  INSTALLSTATE = Longint;
const
  INSTALLSTATE_INVALIDARG = -2;  { An invalid parameter was passed to the function. }
  INSTALLSTATE_UNKNOWN = -1;     { The product is neither advertised or installed. }
  INSTALLSTATE_ADVERTISED = 1;   { The product is advertised but not installed. }
  INSTALLSTATE_ABSENT = 2;       { The product is installed for a different user. }
  INSTALLSTATE_DEFAULT = 5;      { The product is installed for the current user. }

  VC_2005_REDIST_X86 = '{A49F249F-0C91-497F-86DF-B2585E8E76B7}';
  VC_2005_REDIST_X64 = '{6E8E85E8-CE4B-4FF5-91F7-04999C9FAE6A}';
  VC_2005_REDIST_IA64 = '{03ED71EA-F531-4927-AABD-1C31BCE8E187}';
  VC_2005_SP1_REDIST_X86 = '{7299052B-02A4-4627-81F2-1818DA5D550D}';
  VC_2005_SP1_REDIST_X64 = '{071C9B48-7C32-4621-A0AC-3F809523288F}';
  VC_2005_SP1_REDIST_IA64 = '{0F8FB34E-675E-42ED-850B-29D98C2ECE08}';
  VC_2005_SP1_ATL_SEC_UPD_REDIST_X86 = '{837B34E3-7C30-493C-8F6A-2B0F04E2912C}';
  VC_2005_SP1_ATL_SEC_UPD_REDIST_X64 = '{6CE5BAE9-D3CA-4B99-891A-1DC6C118A5FC}';
  VC_2005_SP1_ATL_SEC_UPD_REDIST_IA64 = '{85025851-A784-46D8-950D-05CB3CA43A13}';

  VC_2008_REDIST_X86 = '{FF66E9F6-83E7-3A3E-AF14-8DE9A809A6A4}';
  VC_2008_REDIST_X64 = '{350AA351-21FA-3270-8B7A-835434E766AD}';
  VC_2008_REDIST_IA64 = '{2B547B43-DB50-3139-9EBE-37D419E0F5FA}';
  VC_2008_SP1_REDIST_X86 = '{9A25302D-30C0-39D9-BD6F-21E6EC160475}';
  VC_2008_SP1_REDIST_X64 = '{8220EEFE-38CD-377E-8595-13398D740ACE}';
  VC_2008_SP1_REDIST_IA64 = '{5827ECE1-AEB0-328E-B813-6FC68622C1F9}';
  VC_2008_SP1_ATL_SEC_UPD_REDIST_X86 = '{1F1C2DFC-2D24-3E06-BCB8-725134ADF989}';
  VC_2008_SP1_ATL_SEC_UPD_REDIST_X64 = '{4B6C7001-C7D6-3710-913E-5BC23FCE91E6}';
  VC_2008_SP1_ATL_SEC_UPD_REDIST_IA64 = '{977AD349-C2A8-39DD-9273-285C08987C7B}';
  VC_2008_SP1_MFC_SEC_UPD_REDIST_X86 = '{9BE518E6-ECC6-35A9-88E4-87755C07200F}';
  VC_2008_SP1_MFC_SEC_UPD_REDIST_X64 = '{5FCE6D76-F5DC-37AB-B2B8-22AB8CEDB1D4}';
  VC_2008_SP1_MFC_SEC_UPD_REDIST_IA64 = '{515643D1-4E9E-342F-A75A-D1F16448DC04}';

  VC_2010_REDIST_X86 = '{196BB40D-1578-3D01-B289-BEFC77A11A1E}';
  VC_2010_REDIST_X64 = '{DA5E371C-6333-3D8A-93A4-6FD5B20BCC6E}';
  VC_2010_REDIST_IA64 = '{C1A35166-4301-38E9-BA67-02823AD72A1B}';
  VC_2010_SP1_REDIST_X86 = '{F0C3E5D1-1ADE-321E-8167-68EF0DE699A5}';
  VC_2010_SP1_REDIST_X64 = '{1D8E6291-B0D5-35EC-8441-6616F567A0F7}';
  VC_2010_SP1_REDIST_IA64 = '{88C73C1C-2DE5-3B01-AFB8-B46EF4AB41CD}';

  { Microsoft Visual C++ 2012 x86 Minimum Runtime - 11.0.61030.0 (Update 4) }
  VC_2012_REDIST_MIN_UPD4_X86 = '{BD95A8CD-1D9F-35AD-981A-3E7925026EBB}';
  VC_2012_REDIST_MIN_UPD4_X64 = '{CF2BEA3C-26EA-32F8-AA9B-331F7E34BA97}';
  { Microsoft Visual C++ 2012 x86 Additional Runtime - 11.0.61030.0 (Update 4)  }
  VC_2012_REDIST_ADD_UPD4_X86 = '{B175520C-86A2-35A7-8619-86DC379688B9}';
  VC_2012_REDIST_ADD_UPD4_X64 = '{37B8F9C7-03FB-3253-8781-2517C99D7C00}';

  { Visual C++ 2013 Redistributable 12.0.21005 }
  VC_2013_REDIST_X86_MIN = '{13A4EE12-23EA-3371-91EE-EFB36DDFFF3E}';
  VC_2013_REDIST_X64_MIN = '{A749D8E6-B613-3BE3-8F5F-045C84EBA29B}';

  VC_2013_REDIST_X86_ADD = '{F8CFEB22-A2E7-3971-9EDA-4B11EDEFC185}';
  VC_2013_REDIST_X64_ADD = '{929FBD26-9020-399B-9A7A-751D61F0B942}';

  { Visual C++ 2015 Redistributable 14.0.23026 }
  VC_2015_REDIST_X86_MIN = '{A2563E55-3BEC-3828-8D67-E5E8B9E8B675}';
  VC_2015_REDIST_X64_MIN = '{0D3E9E15-DE7A-300B-96F1-B4AF12B96488}';

  VC_2015_REDIST_X86_ADD = '{BE960C1C-7BAD-3DE6-8B1A-2616FE532845}';
  VC_2015_REDIST_X64_ADD = '{BC958BD2-5DAC-3862-BB1A-C1BE0790438D}';

  { Visual C++ 2015 Redistributable 14.0.24210 }
  VC_2015_REDIST_X86 = '{8FD71E98-EE44-3844-9DAD-9CB0BBBC603C}';
  VC_2015_REDIST_X64 = '{C0B2C673-ECAA-372D-94E5-E89440D087AD}';

{已校验XPSP3+系统自带这个dll和API}
function MsiQueryProductState(szProduct: string): INSTALLSTATE; 
external 'MsiQueryProductStateW@msi.dll stdcall';

{检查VC++运行库是否安装}
function VCVersionInstalled(const ProductID: string): Boolean;
begin
  Result := MsiQueryProductState(ProductID) = INSTALLSTATE_DEFAULT;
end;

{等待任务信息设置}
procedure SetTaskStatus(msg:String;progressStyle:TNewProgressBarStyle);
begin
    WizardForm.StatusLabel.Caption := msg;
    WizardForm.ProgressGauge.Style := progressStyle;
end;

{比较版本字符串}
// 若 V1 > V2 返回值  1
// 若 V1 = V2 返回值  0
// 若 V1 < V2 返回值 -1
function CompareVersion(V1, V2: string): Integer;
var
  P, N1, N2: Integer;
begin
  Result := 0;
  while (Result = 0) and ((V1 <> '') or (V2 <> '')) do
  begin
    P := Pos('.', V1);
    if P > 0 then
    begin
      N1 := StrToInt(Copy(V1, 1, P - 1));
      Delete(V1, 1, P);
    end
      else
    if V1 <> '' then
    begin
      N1 := StrToInt(V1);
      V1 := '';
    end
      else
    begin
      N1 := 0;
    end;

    P := Pos('.', V2);
    if P > 0 then
    begin
      N2 := StrToInt(Copy(V2, 1, P - 1));
      Delete(V2, 1, P);
    end
      else
    if V2 <> '' then
    begin
      N2 := StrToInt(V2);
      V2 := '';
    end
      else
    begin
      N2 := 0;
    end;

    if N1 < N2 then Result := -1
      else
    if N1 > N2 then Result := 1;
  end;
end;

{反向返回特定字符所在字符串位置}
function LastCharPos(const S:string;const TmpChar:Char): Integer;
var
  i: Integer;
begin
  result := 0;
  for i := length(S) downto 1 do
    if S[i] = TmpChar then
    begin
      result := i;
      break;
    end;
end;

{替换文件扩展名}
//ChangeFileExt('a.exe','com') return a.com
function ChangeFileExt(const FileName:String;const NewExt:String):String;
var
TmpLocal:Integer;
TmpResult:String;
begin
    TmpLocal := LastCharPos(FileName,'.');
    if 0=TmpLocal then begin
      TmpResult:= FileName + '.' + NewExt;
    end else begin
      TmpResult:=Copy(FileName, 1, TmpLocal) + NewExt;
    end;
    Result:=TmpResult;
end;

[/Code]
#endif // ISS_SCRIPT_UTILS
