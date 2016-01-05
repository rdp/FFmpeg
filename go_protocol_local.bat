@rem make ffmpeg_g.exe 
del yo.mp4
echo "this does not doa  rebuild"
del myfile
del ksl.2hr.ts
@rem -analyzeduration 1G -probesize 1G
.\ffmpeg_g.exe -local_buffer_size 500M -debug 1 -loglevel info -receiver_component "Hauppauge WinTV 885 TS Capture" -tune_freq 615250 -dtv a -dump_graph_filename dump.grf -dump_raw_bytes_filename ksl.3hr.ts -i dshowbda:video="Hauppauge WinTV 885 BDA Tuner/Demod" -t 10800 -f null out.null -loglevel info

@rem byutv 651250
@rem ksl 615250