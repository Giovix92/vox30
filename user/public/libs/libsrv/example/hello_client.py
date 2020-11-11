#!/usr/bin/python

from libsrv import *

sox = insrv_client ('finger', 'tcp', 'fingerhosting.com', 0, 0, 0)

print sox

close (sox)
