while (!File.exist?("c:\\Users\\rdp\\Downloads\\ffmpeg.exe"))
  puts "still not there"
  sleep 0.1
end
system("gof.bat")