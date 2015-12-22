rm ffmpeg*exe
make ffmpeg_g.exe -j 8
wine ./ffmpeg_g.exe -i dshowbda://abc
