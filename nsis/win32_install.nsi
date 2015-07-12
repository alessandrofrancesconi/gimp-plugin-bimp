# This is the installer source for: BIMP. Batch Image Manipulation Plugin for GIMP.
# Alessandro Francesconi - http://www.alessandrofrancesconi.it/projects/bimp

!include "MUI2.nsh"
!include "nsDialogs.nsh"

Name "Batch Image Manipulation Plugin for GIMP"
OutFile "gimp-plugin-bimp_win32.exe"
RequestExecutionLevel admin
ShowInstDetails show

Var GIMP_dir
Var GIMP_dir_usr28
Var GIMP_dir_usr26

!define MUI_ICON "icon.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE ..\LICENSE
!insertmacro MUI_PAGE_INSTFILES
Page custom finishDialog

!insertmacro MUI_LANGUAGE "English" ; This is mandatory: at least one language string must be declared

# Before start, check for GIMP installation existence (for both 32 and 64 machines)
Function .onInit

    SetRegView 32 ; try on 32-bit
    ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GIMP-2_is1" InstallLocation

    StrCmp $0 "" 0 GimpFound
        SetRegView 64 ; not found, try on 64-bit instead
        ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GIMP-2_is1" InstallLocation
        
        StrCmp $0 "" 0 GimpFound
        ; not found again, abort
        MessageBox MB_OK "GIMP installation was not found. Please install GIMP before running this installer."
        Abort
    
    GimpFound: ; great!
    StrCpy $GIMP_dir $0
    StrCpy $INSTDIR $GIMP_dir"lib\gimp\2.0\plug-ins" ; fill $INSTDIR with the plugin's directory
    
    StrCpy $GIMP_dir_usr28 $PROFILE\.gimp-2.8\plug-ins ; also fills the user's data folder (to remove old files)
    StrCpy $GIMP_dir_usr26 $PROFILE\.gimp-2.6\plug-ins ; do the same for GIMP 2.6
    
    InitPluginsDir
	File /oname=$PLUGINSDIR\finish.bmp "finish.bmp" ; load custom image for finishDialog

FunctionEnd


Function .onVerifyInstDir
    
  IfFileExists $GIMP_dir\bin\gimp-*.exe PathGood
    Abort ; if $GIMP_dir is not a GIMP directory, don't let us install there
  PathGood:
  
FunctionEnd

# Creates the final dialog, with some useful information
Function finishDialog

	nsDialogs::Create 1018
	Pop $R0
    
    ${NSD_CreateLabel} 0 0 100% 24u "BIMP has been successfully installed!$\nTo use it, open GIMP -> File menu -> $\"Batch Image Manipulation...$\""
    
    ${NSD_CreateBitmap} 0 24u 350 225 ""
    Pop $0
    ${NSD_SetImage} $0 $PLUGINSDIR\finish.bmp $1
        
    nsDialogs::Show
    
    ${NSD_FreeImage} $1
    
FunctionEnd

# first delete any previous installation
Section "Remove old files" SecRemOld
        
    Delete $GIMP_dir_usr28\bimp.exe
    Delete $GIMP_dir_usr26\bimp.exe
    
    RMDir /r $GIMP_dir_usr28\bimp-locale
    RMDir /r $GIMP_dir_usr26\bimp-locale
    
SectionEnd

# This step will copy the necessary files on GIMP's plugin folder ($INSTDIR)
Section "File copy" SecInstall
    
    SetOutPath $INSTDIR
    
    File ..\bin\win32\bimp.exe
    File /r /x *.po ..\bimp-locale

SectionEnd

# BIMP needs pcre3 DLL to be registered on the system
Section "Register DLLs" SecRegisterDLL

    SetOutPath "$WINDIR\System32"
    File ..\bin\win32\pcre3.dll
    
    ExecWait "regsvr32.exe /s $WINDIR\System32\pcre3.dll"
    
SectionEnd
