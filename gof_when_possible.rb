puts 'waiting till there'
while (!File.exist?("c:\\Users\\rdp\\Downloads\\ffmpeg.exe"))
  sleep 0.1
end
system("gof.bat")