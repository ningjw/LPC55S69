blhost.exe -u 0x01fc9,0x0021 -- flash-erase-region 0x00008000 0x00047E00

blhost.exe -u 0x01fc9,0x0021 -- flash-image core0.hex

blhost.exe -u 0x01fc9,0x0021 -- reset

pause