@rem make ffmpeg_g.exe 
del yo.mp4
echo "this does not do a rebuild"
del myfile
del ksl.2hr.ts
@rem -analyzeduration 1G -probesize 1G
.\ffmpeg_g.exe -local_buffer_size 500M -debug 1 -loglevel info -receiver_component "Hauppauge WinTV 885 TS Capture" -atsc_physical_channel 44 -dtv a -dump_graph_filename dump.grf -i dshowbda:video="Hauppauge WinTV 885 BDA Tuner/Demod" -t 10800 -r 10 -s 10x10 -y out.mp4 -loglevel info

@rem -atsc_physical_channel 44 
@rem byutv -tune_freq 651250
@rem ksl 615250 -atsc_physical_channel 38