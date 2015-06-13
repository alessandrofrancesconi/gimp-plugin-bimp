# This is the installer source for: BIMP. Batch Image Manipulation Plugin for GIMP.
# Alessandro Francesconi - http://www.alessandrofrancesconi.it/projects/bimp

!include "MUI2.nsh"

Name "Batch Image Manipulation Plugin for GIMP"
OutFile "gimp-plugin-bimp_win32.exe"
RequestExecutionLevel admin
ShowInstDetails show

Var GIMP_dir

!define MUI_ICON "icon.ico"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE ..\LICENSE
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English" # This is mandatory: at least one language string must be declared

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

FunctionEnd

Function .onVerifyInstDir
  IfFileExists $GIMP_dir\bin\gimp-*.exe PathGood
    Abort ; if $GIMP_dir is not a GIMP directory, don't let us install there
  PathGood:
FunctionEnd

# This step will copy the necessary files on GIMP's plugin folder ($INSTDIR)
Section "File copy" SecDummy
    
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
