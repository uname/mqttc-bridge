#!/usr/bin/env python
#-*- coding: utf-8 -*-

import pty
import os
import select

def mkpty():
    master1, slave = pty.openpty()
    devName = os.ttyname(slave)
    print "device names -> ", devName
    return master1, devName


if __name__ == "__main__":

    master1, devName = mkpty()
    
    while 1:
        rl, wl, el = select.select([master1], [], [], 1)
        if len(rl) < 1:
            continue
            
        data = os.read(master1, 128)
        print "buffer size -> %d\n%s" % (len(data), data)
        os.write(master1, data)

