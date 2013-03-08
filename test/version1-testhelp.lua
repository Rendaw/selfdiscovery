#!/usr/bin/lua
Success, ResultType, Result = 
	os.execute('../variant-debug/app/build/selfdiscovery version1-testhelp-controller.lua Verbose Help')
if not Success then
	print('TEST FAILED: Result ' .. ResultType .. ', ' .. Result)
	return 1
else
	return 0
end
