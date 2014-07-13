#!/usr/bin/python
#
# check for syntax errors in a python script
#

import py_compile
import sys
print py_compile.compile(sys.argv[1])
