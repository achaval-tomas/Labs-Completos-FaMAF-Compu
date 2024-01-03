#!/bin/bash

# ej1
cat /proc/cpuinfo | grep "model name"

# ej2
cat /proc/cpuinfo | grep "model name" | wc -l

# ej3
wget -O - -q https://www.gutenberg.org/files/11/11-0.txt | sed 's/Alice/Tomas/g' > TOMAS_in_wonderland.txt

# ej4
sort -nr -k5 weather_cordoba.in | awk 'NR==1 {print $1, $2, $3}' # date of max max temp
sort -nr -k5 weather_cordoba.in | awk 'END {print $1, $2, $3}'   # date of min max temp

# ej5
sort -n -k3 atpplayers.in

# ej6
awk '{print $0, $7 - $8}' superliga.in | sort -k2nr -k9nr 

# ej7
ip addr | grep -o "ether ..:..:..:..:..:.."

# ej8
# create and save ten .srt files in a new folder.
mkdir SERIE_NO_PIRATEADA && touch SERIE_NO_PIRATEADA/miserie_S0E{1..10}_es.srt
# rename the recently created files removing _es suffix.
for file in SERIE_NO_PIRATEADA/*.srt; do mv $file "${file%_es.srt}.srt" ; done 

# OPCIONAL a)
# cut a 10s clip from second 5 of the video.
ffmpeg -ss 00:00:05 -i screenRec.mp4 -t 00:00:10 trimmedVid.mp4 -loglevel quiet -stats

# trim the first 5 and last 2 seconds of any video.
# ffmpeg -ss 00:00:05 -i doublePenChaos.mp4 cutVid.mp4 && ffmpeg -ss 00:00:00 -i cutVid.mp4 -to $(( $(ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 cutVid.mp4 |cut -d\. -f1) - 2)) trimmedVideo.mp4 && rm cutVid.mp4

# OPCIONAL b)
ffmpeg -i voiceAudio.mp3 -i musicBackground.mp3 -filter_complex amerge=inputs=2 -ac 2 overlappedAudio.mp3 -loglevel quiet -stats
