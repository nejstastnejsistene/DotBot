#!/bin/sh

./screencap.sh
python readscreen.py | bin/DotBot - | python findpath.py | python sendevent.py
