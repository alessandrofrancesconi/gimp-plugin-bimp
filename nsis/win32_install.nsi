# This is the installer source for: BIMP. Batch Image Manipulation Plugin for GIMP.
# Alessandro Francesconi - http://www.alessandrofrancesconi.it/projects/bimp

!include "MUI2.nsh"
!include "nsDialogs.nsh"

!define APPNAME "Batch Image Manipulation Plugin for GIMP"
!define APPID "gimp-plugin-bimp"
!define APPVERSION "2.5"

Name "${APPNAME}"
OutFile "gimp-plugin-bimp_win32.exe"
RequestExecutionLevel admin
ShowInstDetails show

Var GIMP_dir
Var GIMP_dir_usr210
Var GIMP_dir_usr28
Var GIMP_dir_usr26
Var UNINSTDIR

!define MUI_ICON "icon.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE ..\LICENSE
!insertmacro MUI_PAGE_INSTFILES
Page custom finishDialog

!insertmacro MUI_LANGUAGE "English" ; This is mandatory: at least one language string must be declared

# When in silent mode, this function prints Str in the STDOUT
!macro PRINT_SILENT_OUT Str
    IfSilent 0 +9
    
    System::Call 'kernel32::GetStdHandle(i -11)i.r0'
    System::Call 'kernel32::AttachConsole(i -1)i.r1'
    ${If} $0 = 0
    ${OrIf} $1 = 0
        System::Call 'kernel32::AllocConsole()'
        System::Call 'kernel32::GetStdHandle(i -11)i.r0'
    ${EndIf}
    FileWrite $0 "${Str}"
!macroend 

!macro VerifyUserIsAdmin
UserInfo::GetAccountType
pop $0
${If} $0 != "admin" ;Require admin rights on NT4+
        messageBox mb_iconstop "Administrator rights required!"
        setErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
        quit
${EndIf}
!macroend

!macro FillGimpDir UN
Function ${UN}FillGimpDir
   SetRegView 32 ; try on 32-bit
    ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GIMP-2_is1" InstallLocation

    StrCmp $0 "" 0 GimpFound
        SetRegView 64 ; not found, try on 64-bit instead
        ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GIMP-2_is1" InstallLocation
        
        StrCmp $0 "" 0 GimpFound
        ; not found again, ask the user
        MessageBox MB_OK|MB_ICONEXCLAMATION "The installer cannot find GIMP installation by itself. Please click OK and manually select the folder where GIMP is installed." /SD IDOK
        
        ; but if in Silent mode, abort everything
        !insertmacro PRINT_SILENT_OUT "GIMP installation was not found. Please install GIMP before running this.$\n"
        IfSilent 0 +2
        Abort

        ; show the dialog
        nsDialogs::SelectFolderDialog "Select GIMP directory, typically is $\"C:\Program Files\GIMP 2$\"" "C:\Program Files"
        Pop $0
        ; check if user aborted it
        StrCmp $0 "error" 0 GimpFound
        Abort
	
	GimpFound: ; great!
	StrCpy $GIMP_dir $0
FunctionEnd
!macroend
!insertmacro FillGimpDir "" 
!insertmacro FillGimpDir "un."

# Before start, check for GIMP installation existence (for both 32 and 64 machines)
Function .onInit
    !insertmacro PRINT_SILENT_OUT "$\nBIMP. Batch Image Manipulation Plugin for GIMP.$\n"
    
	Call FillGimpDir
    StrCpy $INSTDIR "$GIMP_dir\lib\gimp\2.0\plug-ins" ; fill $INSTDIR with the plugin's directory
    StrCpy $UNINSTDIR "$GIMP_dir\etc\gimp\2.0" ; fill $UNINSTDIR with the plugin's uninstaller
    
    ; if $INSTDIR does not exists, don't let us continue
    IfFileExists $INSTDIR PathGood
        MessageBox MB_OK|MB_ICONEXCLAMATION "Error: invalid GIMP plugins folder $INSTDIR." /SD IDOK
        !insertmacro PRINT_SILENT_OUT "Error: invalid GIMP plugins folder $INSTDIR.$\n"
        Abort
    PathGood:
    
    !insertmacro PRINT_SILENT_OUT "GIMP folder found in $INSTDIR.$\n"
    
    StrCpy $GIMP_dir_usr210 $PROFILE\.gimp-2.10\plug-ins ; also fills the user's data folder (to remove old files)
    StrCpy $GIMP_dir_usr28 $PROFILE\.gimp-2.8\plug-ins ; do the same for GIMP 2.8
    StrCpy $GIMP_dir_usr26 $PROFILE\.gimp-2.6\plug-ins ; do the same for GIMP 2.6
    
    InitPluginsDir
	File /oname=$PLUGINSDIR\finish.bmp "finish.bmp" ; load custom image for finishDialog
FunctionEnd

# called when the installation ends successfully
Function .onInstSuccess
    !insertmacro PRINT_SILENT_OUT "BIMP has been successfully installed.$\n$\n"
FunctionEnd

# Creates the final dialog, with some useful information
Function finishDialog
	nsDialogs::Create 1018
	Pop $R0
    
    ${NSD_CreateLabel} 0 0 100% 24u "BIMP has been successfully installed!$\nTo use it, open GIMP > File menu > $\"Batch Image Manipulation...$\""
    
    ${NSD_CreateBitmap} 0 24u 350 225 ""
    Pop $0
    ${NSD_SetImage} $0 $PLUGINSDIR\finish.bmp $1
    
    nsDialogs::Show
    !insertmacro PRINT_SILENT_OUT "BIMP has been successfully installed.$\n"
    
    ${NSD_FreeImage} $1
FunctionEnd

# first delete any previous installation
Section "Remove old files" SecRemOld
    !insertmacro PRINT_SILENT_OUT "Cleaning up old files...$\n"
    
    Delete $GIMP_dir_usr210\bimp.exe
	Delete $GIMP_dir_usr210\bimp-uninstall.exe
    Delete $GIMP_dir_usr28\bimp.exe
	Delete $GIMP_dir_usr28\bimp-uninstall.exe
    Delete $GIMP_dir_usr26\bimp.exe
    Delete $GIMP_dir_usr26\bimp-uninstall.exe
    Delete $INSTDIR\bimp.exe
	Delete $INSTDIR\bimp-uninstall.exe
    
    RMDir /r $GIMP_dir_usr210\bimp-locale
    RMDir /r $GIMP_dir_usr28\bimp-locale
    RMDir /r $GIMP_dir_usr26\bimp-locale
    RMDir /r $INSTDIR\bimp-locale
SectionEnd

# This step will copy the necessary files on GIMP's plugin folder ($INSTDIR)
Section "File copy" SecInstall
    !insertmacro PRINT_SILENT_OUT "Copying new files...$\n"
    
    SetOutPath $INSTDIR
    
    File ..\bin\win32\bimp.exe
    File /r /x *.po ..\bimp-locale

    # create uninstaller
    WriteUninstaller "$UNINSTDIR\bimp-uninstall.exe"

    # Registry information for add/remove programs
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPID}" "DisplayName" "${APPNAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPID}" "UninstallString" "$\"$UNINSTDIR\bimp-uninstall.exe$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPID}" "QuietUninstallString" "$\"$UNINSTDIR\bimp-uninstall.exe /S$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPID}" "InstallLocation" "$\"$INSTDIR$\""
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPID}" "Publisher" "Alessandro Francesconi"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPID}" "HelpLink" "http://www.alessandrofrancesconi.it/projects/bimp"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPID}" "URLInfoAbout" "http://www.alessandrofrancesconi.it/projects/bimp"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPID}" "DisplayVersion" "${APPVERSION}"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPID}" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPID}" "NoRepair" 1

SectionEnd

# Uninstaller

Function un.onInit
	SetShellVarContext all
 
	# verify the uninstaller - last chance to back out
	MessageBox MB_OKCANCEL "Remove ${APPNAME}?" /SD IDOK IDOK next
		Abort
	next:
	!insertmacro VerifyUserIsAdmin
FunctionEnd

Section "uninstall"
 
	Call un.FillGimpDir
    StrCpy $INSTDIR "$GIMP_dir\lib\gimp\2.0\plug-ins" ; fill $INSTDIR with the plugin's directory
    StrCpy $UNINSTDIR "$GIMP_dir\etc\gimp\2.0" ; fill $UNINSTDIR with the plugin's uninstaller
	
	# Remove files
	Delete $INSTDIR\bimp.exe
	RMDir /r $INSTDIR\bimp-locale
 
	# Always delete uninstaller as the last action
	Delete $UNINSTDIR\bimp-uninstall.exe
 
	# Remove uninstaller information from the registry
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPID}"
SectionEnd
