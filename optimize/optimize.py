import os
import subprocess
import scipy
import scipy.optimize

exe = "./cache_sim"

traces_dir = "./traces"
input_fmt = os.path.join(traces_dir, "%s_trace_aligned.txt")

validation_dir = "./tests/validation"
validation_fmt = os.path.join(validation_dir, "%s_validation.txt")

def avg_access_time(cache_params, stdin, stdout, stderr):
    """" Wrapper for the cache_sim program. It runs the simulation given the
    inputs, parses the output for Average Access Time, and returns it as an
    int. In this way, this function can be used as an objective function to
    optimize. `cache_params` are the arguments to the cache_sim and `stdin`,
    `stdout`, and `stderr` are already-opened files for stdin, stdout, and
    stderr, respectively.
    """
    args = [exe]
    args.extend([str(p) for p in cache_params])
    stdin.seek(0)
    stdout.seek(0)
    stderr.seek(0)
    subprocess.call(args, stdin=stdin, stdout=stdout, stderr=stderr)
    stdout.seek(0)
    for line in stdout:
        if line.startswith("AAT"):
            aat = int(line.split()[1])
            break
    return aat

def cache_area(cache):
    """ Returns the area of a single cache in bits. Includs the data store,
    tag store, and any overehead. Assumes address sizes are 32 bits. `cache` is
    a tuple of (c, b, s)
    """
    c, b, s = [int(x) for x in cache]
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

def constraints(cache_params, *args):
    """ Returns a list in which each element must be >= 0 for a successfully
    optimized problem."""
    c1, b1, s1, c2, b2, s2, c3, b3, s3 = cache_params
    print cache_params
    print total_storage(cache_params)
    return scipy.array([
        (1 << 20) - total_storage(cache_params),
        c1 - s1,
        c2 - s2,
        c3 - s3,
        31 - (c1 - s1),
        31 - (c2 - s2),
        31 - (c3 - s3),
        c1 - s1 - b1,
        c2 - s2 - b2,
        c3 - s3 - b3,
        # c1 - b1 - s1,
        # c2 - b2 - s2,
        # c3 - b3 - s3,
        c1,
        c2 - c1,
        c3 - c2,
        b1,
        b2 - b1,
        b3 - b2,
        s1,
        s2 - s1,
        s3 - s2,
    ])

if __name__ == '__main__':
    tests = [
        {
            "name": "gcc",
            # "params": (9,6,0, 10,6,0, 11,6,0)
            "params": (0,0,0, 0,0,0, 0,0,0)
        },
        {
            "name": "go",
            "params": (3, 2, 0,   4, 2, 1,   5, 2, 2)
        },
        {
            "name": "mcf",
            "params": (9, 6, 2, 10, 6, 3, 11, 6, 4)
        },
    ]
    # for t in tests:
    #     name = t["name"]
    #     params = scipy.array(t["params"])
    #     input = open(input_fmt % name)
    #     output = open(os.path.join("optimize", name + ".out"), "w+")
    #     err = open("/dev/null")
    #     print t["name"]
    #     print "aat:", average_access_time(params, input, output, err)
    #     print "storage:", total_storage(params)
    #     print "constraints:", constraints(params)
    #     print

    name = tests[1]["name"]
    params = scipy.array(tests[0]["params"])
    stdin = open(input_fmt % name)
    stdout = open(os.path.join("optimize", name + ".out"), "w+")
    stderr = open("/dev/null", "w")

    min_aat = float("inf")
    min_size = 0
    min_params = []
    max_size = 1024 * 1024
    for b1 in range(0, 20, 5):
        for c1 in range(b1, 20, 5):
            for s1 in range(0, c1-b1):
                for b2 in range(b1, 20, 5):
                    for c2 in range(max(c1, b2), 20, 5):
                        for s2 in range(s1, c2-b2):
                            for b3 in range(b2, 20, 5):
                                for c3 in range(max(c2, b3), 20, 5):
                                    for s3 in range(s2, c3-b3):
                                        caches = [c1, b1, s1,
                                                  c2, b2, s2,
                                                  c3, b3, s3]
                                        size = total_storage(caches)
                                        if size > max_size:
                                            continue
                                        aat = avg_access_time(caches,
                                                              stdin,
                                                              stdout,
                                                              stderr)
                                        if aat < min_aat:
                                            min_aat = aat
                                            min_size = size
                                            min_params = caches[:]
    print "size:", min_size
    print "aat:", min_aat
    print "params:", min_params


    # xopt = scipy.optimize.fmin_slsqp(func=aat,
    #                                  x0=params,
    #                                  args=(stdin, stdout, stderr),
    #                                  f_ieqcons=constraints,
    #                                  # bounds=[(0, 31)]*len(params),
    #                                  iprint=2, # verbosity
    #                                 )
    # print xopt
    # print "aat:", aat(xopt, stdin, stdout, stderr)
    # print "storage:", total_storage(xopt)
    # print "constraints:", constraints(xopt)
