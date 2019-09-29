#!/usr/bin/env python3

import getopt, sys, os, glob

def usage():
    print()
    print("Usage:")
    for f in glob.glob(os.path.dirname(__file__)+"/generators/*.py"):
        if os.path.isfile(f) and not os.path.basename(f).startswith('_'):
            print("  mcy generate %s" % os.path.basename(f)[:-3])
    print()
    exit(1)

def import_by_string(full_name):
    module_name, unit_name = full_name.rsplit('.', 1)
    return getattr(__import__(module_name, fromlist=['']), unit_name)

def call_plugin(name, argv):
    try:
        my_function = import_by_string('generators.%s.generator_main' % name)
    except:
        print("ERROR: Generator plugin %s not found." % name)
        exit(1)
    my_function(name, argv)

if len(sys.argv) < 2:
    usage()

call_plugin(sys.argv[1], sys.argv[2:])
exit(0)
