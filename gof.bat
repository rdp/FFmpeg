@rem make ffmpeg_g.exe 
echo "using remote ffmpeg.exe"
c:\Users\rdp\Downloads\ffmpeg.exe -f dshow -list_devices 1 -dtv 4 -i x
c:\Users\rdp\Downloads\ffmpeg.exe -debug 1 -loglevel verbose -f dshow -receiver_component "Hauppauge WinTV 885 TS Capture" -tune_freq 651250 -dtv 4 -dump_dtv_graph dump.grf -i video="Hauppauge WinTV 885 BDA Tuner/Demod" -t 0.1 -y yo.mp4