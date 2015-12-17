 # slightly faster than ffmpeg_g.exe with a copy
rm /Volumes/Users/rdp/Downloads/ffmpeg.exe
make ffmpeg.exe -j 8 && cp ffmpeg.exe /Volumes/Users/rdp/Downloads && echo "copied"
