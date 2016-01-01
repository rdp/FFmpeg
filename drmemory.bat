@rem "c:\Program Files (x86)\Dr. Memory\bin\drmemory.exe" -- .\ffmpeg_g.exe -local_buffer_size 50M -debug 1 -loglevel info -receiver_component "Hauppauge WinTV 885 TS Capture" -tune_freq 615250 -dtv a -dump_dtv_graph dump.grf -i dshowbda:video="Hauppauge WinTV 885 BDA Tuner/Demod" -y ksl.mp4

@rem -local_buffer_size 500M

 "c:\Program Files (x86)\Dr. Memory\bin\drmemory.exe" -- .\ffmpeg_g.exe -local_buffer_size 200M -debug 1 -loglevel info -receiver_component "Hauppauge WinTV 885 TS Capture" -tune_freq 615250 -dtv a -dump_dtv_graph dump.grf -i dshowbda:video="Hauppauge WinTV 885 BDA Tuner/Demod" -s 100x100 -t 10 -y out.mp4
 
 @rem -f null out.null