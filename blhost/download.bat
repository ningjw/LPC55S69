blhost.exe -u 0x01fc9,0x0021 -- flash-erase-region 0x00000000 0x00020000

blhost.exe -u 0x01fc9,0x0021 -- flash-image boot.hex

blhost.exe -u 0x01fc9,0x0021 -- flash-image core0.hex

blhost.exe -u 0x01fc9,0x0021 -- reset

pause