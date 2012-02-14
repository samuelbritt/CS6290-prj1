import os
import subprocess

exe = "./cache_sim"

traces_dir = "./traces"
input_fmt = os.path.join(traces_dir, "%s_trace_aligned.txt")

validation_dir = "./tests/validation"
validation_fmt = os.path.join(validation_dir, "%s_validation.txt")

class Cache:
    def __init__(self, c, b, s):
        self.c = c
        self.b = b
        self.s = s

class Validation:
    def __init__(self, program_name, L1, L2, L3):
        self.name = program_name
        self.L1 = L1
        self.L2 = L2
        self.L3 = L3
    def run(self):
        validation = validation_fmt % self.name
        input = input_fmt % self.name
        args = [exe]
        for L in self.L1, self.L2, self.L3:
            args.append(str(L.c))
            args.append(str(L.b))
            args.append(str(L.s))

        print
        print "******* %s Validation *********" % self.name
        print "Validation file:"
        print open(validation).read()
        print
        print "Program output:"
        subprocess.call(args, stdin=open(input))
        print

if __name__ == '__main__':
    Validation("gcc", Cache(9,6,0), Cache(10,6,0), Cache(11,6,0)).run()
