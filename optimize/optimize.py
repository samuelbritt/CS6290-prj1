import os
import subprocess

exe = "./cache_sim"

traces_dir = "./traces"
input_fmt = os.path.join(traces_dir, "%s_trace_aligned.txt")

validation_dir = "./tests/validation"
validation_fmt = os.path.join(validation_dir, "%s_validation.txt")

def aat(cache_params, inputfile):
    """" Wrapper for the cache_sim program. It runs the simulation given the
    inputs, parses the output for Average Access Time, and returns it as an
    int. In this way, this function can be used as an objective function to
    optimize.
    """
    args = [exe]
    args.extend([str(p) for p in cache_params])
    input = open(input_fmt % inputfile)
    output = open(inputfile + ".out", "w+")
    err = open("/dev/null")
    subprocess.call(args, stdin=input, stdout=output, stderr=err)
    output.seek(0)
    for line in output:
        if line.startswith("AAT"):
            aat = int(line.split()[1])
            break
    input.close()
    output.close()
    err.close()
    return aat

if __name__ == '__main__':
    params = (9,6,0, 10,6,0, 11,6,0)
    print aat(params, "gcc")
    params = (3, 2, 0, 4, 2, 1, 5, 2, 2)
    print aat(params, "go")
    params = (9, 6, 2, 10, 6, 3, 11, 6, 4)
    print aat(params, "mcf")
