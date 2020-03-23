"""
    Script originally created by klange (https://github.com/klange/toaruos/blob/master/util/generate_symbols.py")
    I am using it because GCC is fighting with me
    This may be replaced later
    or improved
    or modified
    I honestly don't know
"""
import sys

ignored = ["kernel_symbols_start", "kernel_symbols_end" ]
lines = [ x.strip().split(" ")[2] for x in sys.stdin.readlines() if x not in ignored ]

print(".section .symbols")
for name in lines:
    print(".extern %s" % (name))
    print(".type %s, @function" % (name))

print(".global kernel_symbols_start")
print("kernel_symbols_start:")
for name in lines:
    print(".long %s" % (name))
    print(".asciz \"%s\"" % (name))

print(".global kernel_symbols_end")
print("kernel_symbols_end:")
