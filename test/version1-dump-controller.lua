Discover.Version{Version = 1}

Platform = Discover.Platform()
print('Platform family: ' .. Platform.Family .. ', member: ' .. Platform.Member .. ', arch: ' .. Platform.Arch)

print('Executable location: ' .. Discover.InstallExecutableDirectory{Project = 'PROJECTNAME'}.Location)
print('Library location: ' .. Discover.InstallLibraryDirectory{Project = 'PROJECTNAME'}.Location)
print('Data location: ' .. Discover.InstallDataDirectory{Project = 'PROJECTNAME'}.Location)
print('Config location: ' .. Discover.InstallConfigDirectory{Project = 'PROJECTNAME'}.Location)

Shell = nil
if Platform.Family == 'windows' then
	Shell = Discover.Program{Name = 'cmd.exe'}
else
	Shell = Discover.Program{Name = 'sh'}
end
print('Shell location: ' .. Shell.Location)

CXXCompiler = Discover.CXXCompiler{}
print('C++ compiler location name, path: ' .. CXXCompiler.Name .. ', ' .. CXXCompiler.Path)
CXX11Compiler = Discover.CXXCompiler{CXX11 = true}
print('C++11 compiler location name, path: ' .. CXX11Compiler.Name .. ', ' .. CXX11Compiler.Path)

