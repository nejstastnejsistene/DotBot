#!/bin/sh
adb shell screencap -p /data/local/DotBot/screenshot.png
adb pull /data/local/DotBot/screenshot.png .
