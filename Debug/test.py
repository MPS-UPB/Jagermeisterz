import subprocess
import sys

for argv in sys.argv:
    print argv

def func():
    return "wtf"

#subprocess.check_call(["binarize.exe", "mandrill.pgm", "out"])
#print "this should be commented"
    
print "Finished executing"

a = func()
print a
