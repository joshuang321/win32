Set wshShell = CreateObject("WScript.Shell")
Set wshEnvs = wshShell.Environment("SYSTEM")
wshEnvString = wshEnvs("PYTHONPATH")
ScriptDir = CreateObject("Scripting.FileSystemObject").GetParentFolderName(WScript.ScriptFullName)

isPresent = False
wshEnvStrArr = Split(wshEnvString, ";")
For Each wshEnvStr In wshEnvStrArr

	If ScriptDir = wshEnvStr Then

		isPresent = True
	End If
Next

If Not isPresent Then

	If "" = wshEnvString Then

	wshEnvString = wshEnvString & ScriptDir
	Else
		
		wshEnvString = wshEnvString & ";" & ScriptDir
	End if
	wshEnvs("PYTHONPATH") = wshEnvString
End If