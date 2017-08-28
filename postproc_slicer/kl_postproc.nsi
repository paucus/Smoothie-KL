; kl_postproc.nsi
;
; Kikai Labs slicer postprocessor installer.
;

;--------------------------------

!include "MUI2.nsh"

;--------------------------------

; Installer title
Name "Kikai Labs Slicers Post-Processor"

; Installer File Name
OutFile "klpost_1.0.exe"

SetCompressor lzma

; The default installation directory
InstallDir $PROGRAMFILES\KikaiLabs\SlicerPostProc

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\KikaiLabs\SlicerPostProc" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages
;Page components
;Page directory
;Page instfiles
;UninstPage uninstConfirm
;UninstPage instfiles

  !insertmacro MUI_PAGE_LICENSE "License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "Spanish"

; Translations

;  LangString DESC_SecDummy ${LANG_ENGLISH} "A test section."
;  LangString DESC_SecDummy ${LANG_SPANISH} "Una seccion de prueba."

  ;Assign language strings to sections
;  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
;    !insertmacro MUI_DESCRIPTION_TEXT ${SecDummy} $(DESC_SecDummy)
;  !insertmacro MUI_FUNCTION_DESCRIPTION_END
  
;--------------------------------

; The stuff to install
Section "PostProcessor (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "kl_postproc.py"
  File "GcodeParser.py"
  File "kl_img.py"
  File "kl_vtk_img.py"
  File "VTKRender.py"
  File "kl_postproc.bat"
  File /r "python"
  File "README.txt"
  File "LEAME.txt"
  File "License.txt"
  File /r "imgs"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM "Software\KikaiLabs\SlicerPostProc" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KLSlicerPostProc" "DisplayName" "Kikai Labs Slicers Post-Processor"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KLSlicerPostProc" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KLSlicerPostProc" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KLSlicerPostProc" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd


; Create menu items?
Section "Create Menu Item (optional)"
  CreateDirectory $SMPROGRAMS\KikaiLabs
  CreateDirectory $SMPROGRAMS\KikaiLabs\SlicerPostProc
  CreateShortCut "$SMPROGRAMS\KikaiLabs\SlicerPostProc\Uninstall.lnk" "$INSTDIR\uninstall.exe"
SectionEnd

; Create shortcut?
Section "Create Desktop Shortcut (optional)"
  CreateShortCut "$DESKTOP\Kikai Labs Post Processor.lnk" "$INSTDIR\kl_postproc.bat" "" "$INSTDIR\imgs\kl_postproc_128.ico" 0
SectionEnd

;----------------------------------

; StrContains
; This function does a case sensitive searches for an occurrence of a substring in a string. 
; It returns the substring if it is found. 
; Otherwise it returns null(""). 
; Written by kenglish_hi
; Adapted from StrReplace written by dandaman32
 
 
Var STR_HAYSTACK
Var STR_NEEDLE
Var STR_CONTAINS_VAR_1
Var STR_CONTAINS_VAR_2
Var STR_CONTAINS_VAR_3
Var STR_CONTAINS_VAR_4
Var STR_RETURN_VAR
 
Function StrContains
  Exch $STR_NEEDLE
  Exch 1
  Exch $STR_HAYSTACK
  ; Uncomment to debug
  ;MessageBox MB_OK 'STR_NEEDLE = $STR_NEEDLE STR_HAYSTACK = $STR_HAYSTACK '
    StrCpy $STR_RETURN_VAR ""
    StrCpy $STR_CONTAINS_VAR_1 -1
    StrLen $STR_CONTAINS_VAR_2 $STR_NEEDLE
    StrLen $STR_CONTAINS_VAR_4 $STR_HAYSTACK
    loop:
      IntOp $STR_CONTAINS_VAR_1 $STR_CONTAINS_VAR_1 + 1
      StrCpy $STR_CONTAINS_VAR_3 $STR_HAYSTACK $STR_CONTAINS_VAR_2 $STR_CONTAINS_VAR_1
      StrCmp $STR_CONTAINS_VAR_3 $STR_NEEDLE found
      StrCmp $STR_CONTAINS_VAR_1 $STR_CONTAINS_VAR_4 done
      Goto loop
    found:
      StrCpy $STR_RETURN_VAR $STR_NEEDLE
      Goto done
    done:
   Pop $STR_NEEDLE ;Prevent "invalid opcode" errors and keep the
   Exch $STR_RETURN_VAR  
FunctionEnd
 
!macro _StrContainsConstructor OUT NEEDLE HAYSTACK
  Push `${HAYSTACK}`
  Push `${NEEDLE}`
  Call StrContains
  Pop `${OUT}`
!macroend
 
!define StrContains '!insertmacro "_StrContainsConstructor"'


# Params (in pop order): Config File Path
Function AddPostProcessor
  # Vars: $R0 = File path, $R1 = File handler, $R2 = Temp File name, $R3 = String Buffer, $R4 var to hold whether the post_process string was found, $R5 = Temp File handler
  Exch $R0 #File
  Push $R1
  Push $R2
  Push $R3
  Push $R4
  Push $R5
  
  FileOpen $R1 $R0 "r"
  GetTempFileName $R2
  FileOpen $R5 $R2 "w"
  
  Loop:
    FileRead $R1 $R3
    IfErrors Done
    ${StrContains} $R4 "post_process =" "$R3"
    StrCmp $R4 "" notfound
      FileWrite $R5 "post_process = $INSTDIR\kl_postproc.bat$\r$\n"
      IfErrors errwri
      Goto endstrcmp
    notfound:
      FileWrite $R5 $R3
      IfErrors errwri
    endstrcmp:
      
    Goto Loop
 
  errwri:
    MessageBox MB_OK "Failed to write"
  Done:
    FileClose $R1
    FileClose $R5
    
    Delete "$R0"
    CopyFiles /SILENT "$R2" "$R0"
    Delete "$R2"
  
  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R2
  Pop $R1
  Pop $R0
FunctionEnd

# Params (in pop order): Dir path, extension filter
Function AddPostProcessorToAllConfigFiles
  # Vars: $R0 = Dir Path, $R1 = Dir handler, $R2 = Dir file name
  Exch $R0 #path
  Push $R1
  Push $R2
    ClearErrors
    FindFirst $R1 $R2 "$R0\*.ini"
    Loop:
      IfErrors Done
      
      Push "$R0\$R2"
      Call AddPostProcessor

      FindNext $R1 $R2
      Goto Loop

    Done: 
      FindClose $R1
  Pop $R2
  Pop $R1
  Pop $R0
FunctionEnd

Section "Auto-configure Slic3r (optional)"
  Push "$APPDATA\Slic3r\print" # folder to search in
  Call AddPostProcessorToAllConfigFiles
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\KLSlicerPostProc"
  DeleteRegKey HKLM "Software\KikaiLabs\SlicerPostProc"

  ; Remove files and uninstaller
  Delete $INSTDIR\kl_postproc.py
  Delete $INSTDIR\GcodeParser.py
  Delete $INSTDIR\kl_img.py
  Delete $INSTDIR\kl_vtk_img.py
  Delete $INSTDIR\VTKRender.py
  ; Delete pyc files also (just in case they were compiled)
  Delete $INSTDIR\kl_postproc.pyc
  Delete $INSTDIR\GcodeParser.pyc
  Delete $INSTDIR\kl_img.pyc
  Delete $INSTDIR\kl_vtk_img.pyc
  Delete $INSTDIR\VTKRender.pyc
  
  Delete $INSTDIR\kl_postproc.bat
  RMDir /r "$INSTDIR\imgs"
  RMDir /r "$INSTDIR\python"
  Delete $INSTDIR\README.txt
  Delete $INSTDIR\LEAME.txt
  Delete $INSTDIR\License.txt
  Delete $INSTDIR\uninstall.exe

  RMDir "$INSTDIR"

  Delete "$DESKTOP\Kikai Labs Post Processor.lnk"
  Delete "$SMPROGRAMS\KikaiLabs\SlicerPostProc\Uninstall.lnk"
  RMDir "$SMPROGRAMS\KikaiLabs\SlicerPostProc"
  RMDir "$SMPROGRAMS\KikaiLabs"
SectionEnd
