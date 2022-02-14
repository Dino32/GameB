del C:\Users\harveynorman\source\repos\GameB\x64\Debug\Assets\ /Q
xcopy C:\Users\harveynorman\source\repos\GameB\Assets C:\Users\harveynorman\source\repos\GameB\x64\Debug\Assets /i

del Assets.dat
del CopyAssets.log

REM -- BITMAPS --
MyMiniz.exe Assets.dat + .\Assets\6x7font.bmpx >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Hero_Suit0_Down_Standing.bmpx >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Hero_Suit0_Down_Walk1.bmpx >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Hero_Suit0_Down_Walk2.bmpx >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Hero_Suit0_Left_Standing.bmpx >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Hero_Suit0_Left_Walk1.bmpx >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Hero_Suit0_Left_Walk2.bmpx >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Hero_Suit0_Right_Standing.bmpx >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Hero_Suit0_Right_Walk1.bmpx >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Hero_Suit0_Right_Walk2.bmpx >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Hero_Suit0_Up_Standing.bmpx >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Hero_Suit0_Up_Walk1.bmpx >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Hero_Suit0_Up_Walk2.bmpx >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Maps\Overworld01.bmpx >> CopyAssets.log

REM -- SOUNDS --
MyMiniz.exe Assets.dat + .\Assets\MenuChooser.wav >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\MenuSelect.wav >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Fading.wav >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Overworld01.ogg >> CopyAssets.log
MyMiniz.exe Assets.dat + .\Assets\Splash.wav >> CopyAssets.log

REM -- TILEMAPS --
MyMiniz.exe Assets.dat + .\Assets\Maps\Overworld01.tmx >> CopyAssets.log
