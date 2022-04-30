ECHO STRINGIFY( >Version.txt
p4 changes -s submitted -m 1 //Projects2/Y2020D-Y2-DataFlow/FracTowerDefenseSolution/...
if %ERRORLEVEL% NEQ 0 (
	ECHO Change 000000 >>Version.txt
) else (
	p4 changes -s submitted -m 1 //Projects2/Y2020D-Y2-DataFlow/FracTowerDefenseSolution/... >>Version.txt
	)
ECHO ) >>Version.txt

exit /b 0