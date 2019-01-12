# ==============================================================================
# read_phoneix_hdr.py
#
# Tools to read an MSTAR Phoenix ASCII header file
#
# Author: J. Pualoa
# ==============================================================================
import os

START_FILE = '[PhoenixHeaderVer'
END_FILE = '[EndofPhoenixHeader]'

def valid_header(contents):
    """Checks file contents for header start and end"""
    start = contents.find(START_FILE)
    end = contents.find(END_FILE)
    if (start < 0) or (end < 0):
        return False
    else:
        return True

def read_header(filename):
    if not os.path.exists(os.path.expandvars(filename)):
        raise ValueError("file %s not found" % filename)
    with open(filename, 'r') as f:
        contents = f.read()
    return contents

def parse_header(filename):
    """Parses Phoenix Header file and returns a dictionary of metadata.

    NOTE: All values are strings and must be converted to appropriate format
          by the application.
    """
    contents = read_header(filename)
    if not valid_header(contents):
        raise ValueError("file %s is not a valid Phoenix Header file" % filename)
    # Separate line entries into list
    lines = contents.strip().split('\n')
    header = {}
    # Parse key/values in each line
    for l in lines[1:-1]: # Remove start and end markings
        key, value = l.split('= ')
        header[key] = value
    return header
