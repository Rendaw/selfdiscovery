Assert = function(Received, Member, Expected)
	if not Received then return end
	if Received[Member] ~= Expected 
	then 
		error('ERROR: Expected \"' .. tostring(Expected) .. '\", received \"' .. tostring(Received[Member]) .. '\".') 
	end
end

AssertGreaterEquals = function(Received, Member, Limit)
	if not Received then return end
	if Received[Member] < Limit
	then 
		error('ERROR: Received \"' .. tostring(Received[Member]) .. '\", which was expected to be >= ' .. Limit .. '.') 
	end
end

AssertGreaterEquals(Discover.Version{Version = 1}, 'Version', 1)

Platform = Discover.Platform()
Assert(Platform, 'Family', 'linux')
Assert(Platform, 'Member', 'ubuntu')

Assert(Discover.InstallExecutableDirectory{Project = 'PROJECTNAME'}, 'Location', '/usr/bin')
Assert(Discover.InstallLibraryDirectory{Project = 'PROJECTNAME'}, 'Location', '/usr/lib')
Assert(Discover.InstallDataDirectory{Project = 'PROJECTNAME'}, 'Location', '/usr/share/PROJECTNAME')
Assert(Discover.InstallConfigDirectory{Project = 'PROJECTNAME'}, 'Location', '/usr/etc')

Assert(Discover.Program{Name = 'sh'}, 'Location', '/bin/sh')

CXXCompiler = Discover.CXXCompiler{}
Assert(CXXCompiler, 'Name', 'g++')
Assert(CXXCompiler, 'Path', '/usr/bin/g++')

CXX11Compiler = Discover.CXXCompiler{CXX11 = true}
Assert(CXXCompiler, 'Name', 'g++')
Assert(CXXCompiler, 'Path', '/usr/bin/g++')

Assert({Result = Discover.CLibrary{Name = 'Missing C library', Optional = true}}, 'Result', nil)

