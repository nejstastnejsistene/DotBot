#!/usr/bin/env python

import plistlib
import commands

package = 'com.nerdyoctopus.gamedots'
name = '{}.plist'.format(package)
path = '/data/data/{}/files/Library/Preferences/{}'.format(package, name)
path2 = '/data/local/DotBot/' + name
activity = '{}/com.apportable.activity.VerdeActivity'.format(package)
num_powerups = 2**31 - 1

def getoutput(cmd):
    print cmd
    return commands.getoutput(cmd)


if __name__ == '__main__':
    import sys
    if len(sys.argv) > 1:
        num_powerups = int(sys.argv[1])


    # Kill Dots if it's running.
    ps = getoutput('adb shell ps')
    for line in ps.split('\n'):
        if package in line:
            pid = line.split()[1]
            getoutput("adb shell su -c 'kill {}'".format(pid))

    # Pull the database.
    print getoutput("adb shell su -c 'cp {} {}'".format(path, path2))
    print getoutput("adb shell su -c 'chown shell:shell {}'".format(path2))
    print getoutput('adb pull {} {}'.format(path2, name))

    # Give us lots of powerups!
    db = plistlib.readPlist(name)
    db['number_of_time_freezes'] = num_powerups
    db['number_of_shrinkers'] = num_powerups
    db['number_of_expanders'] = num_powerups
    plistlib.writePlist(db, name)

    # Push the modified database.
    print getoutput('adb push {} {}'.format(name, path2))
    print getoutput("adb shell su -c 'chown root:root {}'".format(path2))
    print getoutput("adb shell su -c 'cp {} {}'".format(path2, path))

    # Restart the activity.
    print getoutput('adb shell am start {}'.format(activity))
