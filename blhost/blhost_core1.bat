blhost.exe -u 0x01fc9,0x0021 -- flash-erase-region 0x00008000 0x00008000

blhost.exe -u 0x01fc9,0x0021 -- flash-image core1.hex

blhost.exe -u 0x01fc9,0x0021 -- reset

pause