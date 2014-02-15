#!/usr/bin/env python

import plistlib
from commands import getoutput

package = 'com.nerdyoctopus.gamedots'
name = '.Defaults.plist'
path = '/data/data/{}/files/{}'.format(package, name)
activity = '{}/com.apportable.activity.VerdeActivity'.format(package)
num_powerups = 2**31 - 1

if __name__ == '__main__':
    import sys
    if len(sys.argv) > 1:
        num_powerups = int(sys.argv[1])


    # Kill Dots if it's running.
    ps = getoutput('adb shell ps')
    for line in ps.split('\n'):
        if package in line:
            pid = line.split()[1]
            getoutput('adb shell kill {}'.format(pid))

    # Pull the database.
    getoutput('adb pull {} {}'.format(path, name))

    # Give us lots of powerups!
    db = plistlib.readPlist(name)
    db['dots']['number_of_time_freezes'] = num_powerups
    db['dots']['number_of_shrinkers'] = num_powerups
    db['dots']['number_of_expanders'] = num_powerups
    plistlib.writePlist(db, name)

    # Push the modified database.
    getoutput('adb push {} {}'.format(name, path))

    # Restart the activity.
    getoutput('adb shell am start {}'.format(activity))
