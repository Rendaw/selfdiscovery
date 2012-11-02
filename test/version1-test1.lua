#!/usr/bin/lua
Success, ResultType, Result = 
	os.execute('../variant-debug/app/build/selfdiscovery version1-test1-controller.lua' ..
	' Flag1 Flag2=\"Flag 2\nvalue\"' ..
	' PlatformFamily=linux PlatformMember=debian Arch=32' ..
	' InstallExecutableDirectory="Executable location"' ..
	' InstallLibraryDirectory="Library location"' ..
	' InstallDataDirectory="Data location"' ..
	' InstallConfigDirectory="Config location"' ..
	' "Program-Valid program=Valid program location"' ..
	' CXXCompiler="/C++ compiler location/C++ compiler"' ..
	' CXXCompilerClass="C++ compiler class"' ..
	'')
if not Success then
	print('TEST FAILED: Result ' .. ResultType .. ', ' .. Result)
	return 1
else
	return 0
end
