Set wshShell = CreateObject("WScript.Shell")
Set wshEnvs = wshShell.Environment("SYSTEM")
wshEnvString = wshEnvs("PYTHONPATH")
ScriptDir = CreateObject("Scripting.FileSystemObject").GetParentFolderName(WScript.ScriptFullName)
ScriptDir = LCase(ScriptDir)

wshEnvStrArr = Split(wshEnvString, ";")
wshNewEnvStr = ""
For Each wshEnvStr In wshEnvStrArr

   wshEnvStr = LCase(wshEnvStr)
   If ScriptDir <> wshEnvStr Then

      If "" = wshNewEnvStr Then
         wshNewEnvStr = wshNewEnvStr & wshEnvStr

      Else 

         wshNewEnvStr = wshNewEnvStr & ";" & wshEnvStr
      End If
   End If
Next
wshEnvs("PYTHONPATH") = wshNewEnvStr

MsgBox "Path removed successfully!", vbOkOnly OR vbApplicationModal, "uninstall"