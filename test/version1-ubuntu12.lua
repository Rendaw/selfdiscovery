#!/usr/bin/lua
Success, ResultType, Result = os.execute('../variant-debug/app/build/selfdiscovery version1-ubuntu12-controller.lua')
if not Success then
	print('TEST FAILED: Result ' .. ResultType .. ', ' .. Result)
	return 1
else
	return 0
end
