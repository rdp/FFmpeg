@rem make ffmpeg_g.exe 
echo "using remote ffmpeg.exe"
@rem c:\Users\rdp\Downloads\ffmpeg.exe -f dshow -list_devices 1 -dtv 4 -i x
del yo.mp4
del myfile
"C:\Program Files\Git\usr\bin\gzip.exe" -f -d c:\Users\rdp\Downloads\ffmpeg.exe.gz 
c:\Users\rdp\Downloads\ffmpeg.exe -analyzeduration 1G -probesize 1G -debug 1 -loglevel debug -f dshow -receiver_component "Hauppauge WinTV 885 TS Capture" -tune_freq 651250 -dtv 4 -dump_dtv_graph dump.grf -i video="Hauppauge WinTV 885 BDA Tuner/Demod" -t 1 -y yo.mp4