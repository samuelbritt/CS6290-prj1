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
    output = open(os.path.join("optimize", inputfile + ".out"), "w+")
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

def cache_area(cache):
    """ Returns the area of a single cache in bits. Includs the data store,
    tag store, and any overehead. Assumes address sizes are 32 bits. `cache` is
    a tuple of (c, b, s)
    """
    c, b, s = cache
    entry_count = 1 << (c - b)
    tag_size = 32 - (c - s)
    data_store = (1 << c) * 8
    # add 2 for valid/dirty bits
    return data_store + entry_count * (tag_size + 2)

def total_storage(cache_params):
    """ Returns the total area of the cache, in bytes, including the data
    store, tag store, and any overhead.
    """
    storage = 0
    for i in range(0, len(cache_params), 3):
        storage += cache_area(cache_params[i:i + 3])
    return storage / 8


if __name__ == '__main__':
    tests = [
        {
            "name": "gcc",
            "params": (9,6,0, 10,6,0, 11,6,0)
        },
        {
            "name": "go",
            "params": (3, 2, 0, 4, 2, 1, 5, 2, 2)
        },
        {
            "name": "mcf",
            "params": (9, 6, 2, 10, 6, 3, 11, 6, 4)
        },
    ]
    for t in tests:
        print t["name"]
        print "aat:", aat(t["params"], t["name"])
        print "storage:", total_storage(t["params"])
        print
