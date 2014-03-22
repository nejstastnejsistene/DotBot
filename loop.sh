#!/bin/sh

ip=$1

connect() {
    output=`adb devices | grep $ip`
    if [ -z "$output" ]; then
        echo 'connecting...'
        #adb kill-server
        #adb start-server
        adb connect $ip
    fi
}

connect
while true; do
    ./main.py || connect
    sleep 0.5
done
