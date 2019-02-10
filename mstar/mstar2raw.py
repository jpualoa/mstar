# ==============================================================================
# mstar2raw.py
#
# Python wrapper for MSTAR C executable to convert MSTAR native format image
# files to raw binary data files.  For more information on the AFRL MSTAR effort
# visit:
#
#   https://www.sdms.afrl.af.mil/index.php?collection=mstar
#
# Author: J. Pualoa
# ==============================================================================
import os
import sys
import glob
import subprocess


MSTAR2RAW_PATH = os.path.expandvars("$MSTAR_RAW")
MSTAR2RAW = os.path.join(MSTAR2RAW_PATH, 'mstar2raw')


def convert_file(in_file, output_dir, mag_only=False):
    """Converts MSTAR data file to a RAW binary data file and an ASCII
    Phoenix header file
    
    Args:

        in_file (str)
            name of file to be converted

        output_dir (str)
            name of directory to write output to

        mag_only (bool)
            True: MAG only
            False: MAG+PHASE
    """
    os.chdir(output_dir)
    args = [MSTAR2RAW, in_file, int(mag_only)]
    p = subprocess.Popen(args)
    ret_code = p.wait()
    return ret_code
