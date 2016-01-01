@rem make ffmpeg_g.exe 
del yo.mp4
echo "this does not doa  rebuild"
del myfile
del ksl.ts
@rem -analyzeduration 1G -probesize 1G
.\ffmpeg_g.exe -local_buffer_size 500M -debug 1 -loglevel info -receiver_component "Hauppauge WinTV 885 TS Capture" -tune_freq 651250 -dtv a -dump_dtv_graph dump.grf -dump_raw_bytes_file ksl.2hr.ts -i dshowbda:video="Hauppauge WinTV 885 BDA Tuner/Demod" -t 7200 -f null out.null -loglevel verbose

@rem byutv 651250
@rem ksl 615250