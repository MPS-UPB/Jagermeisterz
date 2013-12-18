"""
    This script should be in the same folder
    with the binarization executable.
"""

import subprocess
import sys
import os
import binascii
import time

def argumentsValidation():
    print "Checking if executable exists..."
    if(os.path.isfile(sys.argv[1])):
        print "Success!"
    else:
        print "Executable does not exist."
        print "Program will now exit."
        return False

    print "Checking if input folder exists..."
    if(os.path.exists(sys.argv[2])):
        print "Success!"
    else:
        print "Input image does not exist."
        print "Program will now exit."
        return False

    return True

def executeBinarization():
    for (dirpath, dirnames, filenames) in os.walk(sys.argv[2]):
        for filename in filenames:
            print "Executing binarization for image <", filename ,">..."
            start = time.time()
            subprocess.check_call([sys.argv[1], sys.argv[2] + '\\' + filename, sys.argv[3], sys.argv[4]])
            print "Binarization complete!"

            verifyOutput();
            end = time.time()
            print "Elapsed time: {0:.3f}s".format(end-start)
            
            cleanOutput();
            
    

def verifyOutput():
    print "Checking output..."
    outputImage = sys.argv[3] + ".TIF"
    confidenceImage = sys.argv[4] + ".TIF"

    if(os.path.isfile(outputImage)):
        print "Binary image has been created!"
    else:
        print "Failed to create binary image."
        print "Program will now exit."
        sys.exit()

    if(os.path.isfile(confidenceImage)):
        print "Confidence image has been created!"
    else:
        print "Failed to create confidence image."
        print "Program will now exit."
        sys.exit()

def cleanOutput():
    print "Removing output images..."
    outputImage = sys.argv[3] + ".TIF"
    confidenceImage = sys.argv[4] + ".TIF"
    os.remove(outputImage)
    os.remove(confidenceImage)
    print "Test finished"
    print ""

if __name__=="__main__":
    if len(sys.argv) != 5:
        print "Usage: ", sys.argv[0], "<binarization_exec> <input_images_directory> <output_image_name> <confidence_image_name>"
        print "<binarization_exec> -> Name of the binarization executble"
        print "<input_image_name>  -> Name of the folder containing the images to be binarized"
        print "<output_image_name> -> Name of the binary image"
        print "<confidence_image_name> -> Name of the confidence image"
        sys.exit()

    if (not argumentsValidation()):
        sys.exit()

    executeBinarization();
    
