ffmpeg_g -list_options 1 -f dshow -i video="SMI Grabber Device"
ffmpeg_g -f dshow -i video="SMI Grabber Device":audio="SMI Grabber Device" -y smi.mp4
ffmpeg_g  -f dshow -crossbar_video_input_pin_number 2 -crossbar_audio_input_pin_number 5 -i video="SMI Grabber Device":audio="SMI Grabber Device" -y out.mp4