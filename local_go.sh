make ffmpeg_g.exe -j 8 || rm ffmpeg*exe
wine ./ffmpeg_g.exe -loglevel debug -dtv a -receiver_component "my awesome receiver comopnent" -i dshowbda:audio=x:video=y
