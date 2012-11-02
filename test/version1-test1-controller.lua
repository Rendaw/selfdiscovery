Assert = function(Received, Member, Expected)
	if not Received then return end
	if Received[Member] ~= Expected 
	then 
		error('ERROR: Expected \"' .. tostring(Expected) .. '\", received \"' .. tostring(Received[Member]) .. '\".') 
	end
end

AssertNotEmpty = function(Received, Member)
	if not Received then return end
	if Received[Member] == ""
	then 
		error('ERROR: Received \"' .. tostring(Received[Member]) .. '\", which was expected to be have a value.') 
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

Assert(Discover.Flag{Name = 'Flag1'}, 'Present', true)

Present2 = Discover.Flag{Name = 'Flag2'}
Assert(Present2, 'Present', true)
Assert(Present2, 'Value', 'Flag 2\nvalue')

Absent = Discover.Flag{Name = 'Flag3'}
Assert(Absent, 'Present', false)

Platform = Discover.Platform()
Assert(Platform, 'Family', 'linux')
Assert(Platform, 'Member', 'debian')
Assert(Platform, 'Arch', 32)

Assert(Discover.InstallExecutableDirectory{Project = 'version1-test1'}, 'Location', 'Executable location')
Assert(Discover.InstallLibraryDirectory{Project = 'version1-test1'}, 'Location', 'Library location')
Assert(Discover.InstallDataDirectory{Project = 'version1-test1'}, 'Location', 'Data location')
Assert(Discover.InstallConfigDirectory{Project = 'version1-test1'}, 'Location', 'Config location')

Assert(Discover.Program{Name = 'Valid program'}, 'Location', 'Valid program location')
Assert({Result = Discover.Program{Name = 'Missing program', Optional = true}}, 'Result', nil)

Assert({Result = Discover.CLibrary{Name = 'Missing C library', Optional = true}}, 'Result', nil)


