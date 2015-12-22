 # slightly faster than ffmpeg_g.exe with a copy
rm -f ffmpeg.exe
rm -f /Volumes/Users/rdp/Downloads/ffmpeg.exe
make ffmpeg.exe -j 8 
rm -f ffmpeg.exe.gz
gzip ffmpeg.exe
echo 'copying' 
cp ffmpeg.exe.gz /Volumes/Users/rdp/Downloads/ffmpeg.exe.gz.tmp
mv /Volumes/Users/rdp/Downloads/ffmpeg.exe.gz.tmp /Volumes/Users/rdp/Downloads/ffmpeg.exe.gz # complete so done don't want poller on the receiving side to use it till complete 
