blhost.exe -u 0x01fc9,0x0021 -- flash-erase-region 0x00000000 0x00080000

blhost.exe -u 0x01fc9,0x0021 -- flash-image audio.out

blhost.exe -u 0x01fc9,0x0021 -- reset

pause