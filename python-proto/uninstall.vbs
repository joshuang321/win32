Set objShell = CreateObject("Shell.Application")
ScriptDir = CreateObject("Scripting.FileSystemObject").GetParentFolderName(WScript.ScriptFullName)
objShell.ShellExecute "wscript.exe", Chr(34) & ScriptDir & "\unenviro.vbs" &_
	Chr(34) & " uac", "", "runas", 1