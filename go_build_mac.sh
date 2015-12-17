 # slightly faster than ffmpeg_g.exe with a copy
rm ffmpeg.exe
rm /Volumes/Users/rdp/Downloads/ffmpeg.exe
make ffmpeg.exe -j 8 && cp ffmpeg.exe /Volumes/Users/rdp/Downloads/ffmpeg.exe.tmp
mv /Volumes/Users/rdp/Downloads/ffmpeg.exe.tmp /Volumes/Users/rdp/Downloads/ffmpeg.exe # complete so done don't want poller on the receiving side to use it till complete 
