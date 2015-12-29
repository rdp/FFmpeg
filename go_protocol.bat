@rem make ffmpeg_g.exe 
echo "using remote ffmpeg.exe"
@rem c:\Users\rdp\Downloads\ffmpeg.exe -f dshow -list_devices 1 -dtv 4 -i x
del yo.mp4
del myfile
"C:\Program Files\Git\usr\bin\gzip.exe" -f -d c:\Users\rdp\Downloads\ffmpeg.exe.gz 
@rem -analyzeduration 1G -probesize 1G
c:\Users\rdp\Downloads\ffmpeg.exe -local_buffer_size 500M -debug 1 -loglevel debug -receiver_component "Hauppauge WinTV 885 TS Capture" -tune_freq 651250 -dtv a -dump_dtv_graph dump.grf -i dshowbda:video="Hauppauge WinTV 885 BDA Tuner/Demod" -t 20 -y yo.mp4