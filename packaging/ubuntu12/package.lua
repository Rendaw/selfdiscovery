#!/usr/bin/lua
dofile '../../info.include.lua'

local Variant = '../../variant-release'
if arg[1] and arg[1] == 'debug' then Variant = '../../variant-debug' end

if not os.execute('test -d ' .. Variant)
then
	error('You must have a build in variant directory ' .. Variant .. ' for this to work.')
end

os.execute('mkdir -p ' .. Info.PackageName .. '/DEBIAN')
io.open(Info.PackageName .. '/DEBIAN/control', 'w+'):write([[
Package: ]] .. Info.PackageName .. [[

Version: ]] .. Info.Version .. [[

Section: Development
Priority: Optional
Architecture: all
Depends: libstdc++6 (>= 4.7.0-7ubuntu3)
Maintainer: ]] .. Info.Author .. ' <' .. Info.EMail .. [[>
Description: ]] .. Info.ExtendedDescription .. [[

Homepage: ]] .. Info.Website .. [[

]]):close()

os.execute('mkdir -p ' .. Info.PackageName .. '/usr/bin')
os.execute('cp ' .. Variant .. '/app/build/selfdiscovery ' .. Info.PackageName .. '/usr/bin')
os.execute('mkdir -p ' .. Info.PackageName .. '/usr/share/doc/selfdiscovery')
os.execute('cp ../../license.txt ' .. Info.PackageName .. '/usr/share/doc/' .. Info.PackageName)
os.execute('cp ../../lualicense.txt ' .. Info.PackageName .. '/usr/share/doc/' .. Info.PackageName)
os.execute('dpkg --build ' .. Info.PackageName .. ' .')
os.execute('rm -r ' .. Info.PackageName)

