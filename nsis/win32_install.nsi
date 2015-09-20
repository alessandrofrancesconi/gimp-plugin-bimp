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

# Before start, check for GIMP installation existence (for both 32 and 64 machines)
Function .onInit
    !insertmacro PRINT_SILENT_OUT "$\nBIMP. Batch Image Manipulation Plugin for GIMP.$\n"
    
    SetRegView 32 ; try on 32-bit
    ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GIMP-2_is1" InstallLocation

    StrCmp $0 "" 0 GimpFound
        SetRegView 64 ; not found, try on 64-bit instead
        ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\GIMP-2_is1" InstallLocation
        
        StrCmp $0 "" 0 GimpFound
        ; not found again, abort
        MessageBox MB_OK|MB_ICONEXCLAMATION "GIMP installation was not found. Please install GIMP before running this installer." /SD IDOK
        !insertmacro PRINT_SILENT_OUT "GIMP installation was not found. Please install GIMP before running this installer.$\n"
        Abort
    
    GimpFound: ; great!
    StrCpy $GIMP_dir $0
    StrCpy $INSTDIR $GIMP_dir"lib\gimp\2.0\plug-ins" ; fill $INSTDIR with the plugin's directory
    
    ; if $INSTDIR does not exists, don't let us continue
    IfFileExists $INSTDIR PathGood
        MessageBox MB_OK|MB_ICONEXCLAMATION "Error: can't find folder $INSTDIR." /SD IDOK
        !insertmacro PRINT_SILENT_OUT "Error: can't find folder $INSTDIR.$\n"
        Abort
    PathGood:
    
    !insertmacro PRINT_SILENT_OUT "GIMP folder found in $INSTDIR.$\n"
    
    StrCpy $GIMP_dir_usr28 $PROFILE\.gimp-2.8\plug-ins ; also fills the user's data folder (to remove old files)
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
    
    Delete $GIMP_dir_usr28\bimp.exe
    Delete $GIMP_dir_usr26\bimp.exe
    
    RMDir /r $GIMP_dir_usr28\bimp-locale
    RMDir /r $GIMP_dir_usr26\bimp-locale
SectionEnd

# This step will copy the necessary files on GIMP's plugin folder ($INSTDIR)
Section "File copy" SecInstall
    !insertmacro PRINT_SILENT_OUT "Copying new files...$\n"
    
    SetOutPath $INSTDIR
    
    File ..\bin\win32\bimp.exe
    File /r /x *.po ..\bimp-locale
SectionEnd

# BIMP needs pcre3 DLL to be registered on the system
Section "Register DLLs" SecRegisterDLL
    !insertmacro PRINT_SILENT_OUT "Registering pcre3...$\n"

    SetOutPath "$WINDIR\System32"
    File ..\bin\win32\pcre3.dll
    
    ExecWait "regsvr32.exe /s $WINDIR\System32\pcre3.dll"
SectionEnd
