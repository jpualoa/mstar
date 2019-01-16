# ==============================================================================
# mstar2jpeg.py
#
# Python wrapper for MSTAR C executable to convert MSTAR native format image
# files to JPEG.  For more information on the AFRL MSTAR effort visit:
#
#	https://www.sdms.afrl.af.mil/index.php?collection=mstar
#
# Author: J. Pualoa
# ==============================================================================
import os
import sys
import glob
import subprocess

MSTAR2JPEG = os.path.join(os.path.expandvars("$MSTAR2JPEG_PATH"), 'mstar2jpeg')

def convert_file(in_file, contrast_enhance, compression_quality,
                 dump_header, verbose):
    """Converts MSTAR data file to JPEG"""
    jpeg_file = in_file + '.jpg' # append JPEG extension
    print(in_file, jpeg_file)
    args = [MSTAR2JPEG, '-i', in_file, '-o', jpeg_file, '-q', str(compression_quality)]
    if contrast_enhance: args.append('-e')
    if verbose: args.append('v')
    if dump_header: args.append('h')
    subprocess.Popen(args)
    return jpeg_file


def convert_image_clutter(in_file, dep_angle, source,
                          contrast_enhance=True, compression_quality=75,
                          dump_header=False, verbose=False):
    """Convert mstar clutter image to JPEG and return metadata dictionary"""
    result = convert_file(in_file, contrast_enhance,
                          compression_quality, dump_header, verbose)
    d = {}
    d['filename'] = result
    d['depression_angle'] = dep_angle
    d['source'] = source
    return d


def convert_image_target(in_file, dep_angle, target_class,
                         target_instance, contrast_enhance=True, compression_quality=75,
                         dump_header=False, verbose=False):
    """Convert mstar target image to JPEG and return metadata dictionary"""
    result = convert_file(in_file, contrast_enhance=contrast_enhance,
                          compression_quality=compression_quality,
                          dump_header=dump_header, verbose=verbose)
    d = {}
    d['filename'] = result
    d['depression_angle'] = dep_angle
    d['target_class'] = target_class
    d['target_instance'] = target_instance
    return d
