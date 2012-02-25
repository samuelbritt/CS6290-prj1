Sam Britt
=========

CS 6290 Project 1
-----------------

To build:

 - Type `make` to build the cache simulation application in the current
   directory.
 - Type `make test` to build and run the tests in the `tests/`
   directory.
 - Type `make clean` to remove all compiled files.

Validation Requirement:

 - All the trace files provided on T-Square are in the `traces/`
   directory.
 - All the validation results provided on T-Square are in the
   `tests/validation/` directory, along with shell scripts to run the
   validation tests.
 - Results of the validation runs are in the same directory, with file
   extension `.res`. Alternatively, run the scripts (from the project
   root) and compare the `diff` output. The only output from `diff`
   should be the AAT off by at most 1 due to rounding.
 - Final cache contents after the validation runs are in the same
   directory, with file extention `.contents`. The file format is a
   header for the cache, followed by the adresses in the data store.
   Each set is separated by a newline, and each block within a set is
   separated by a `|` symbol. Each block is preceded by a `V` and/or a
   `D` if the block is valid and/or dirty, respectively, at the end of
   the trace.

To run:

 - The `cache_sim` program takes 9 integers as arguments, which are the
   parameters of the three caches to simulate; that is, run as
       cache_sim c1 b1 s1 c2 b2 s2 c3 b3 s3
 - The `cache_sim` program reads the trace of memory acesses to simulate
   from `stdin`, and writes simulation statistics to `stdout`. It also
   writes the final contents of the cache to `stderr`, which can
   dominate your terminal for large caches. Therefore, you might want to
   run it as
       cache_sim c1 b1 s1 c2 b2 s2 c3 b3 s3 < input_trace 2> cache_contents_file
   where `cache_contents_file` can be `/dev/null` if you are not
   interested in the final contents.
 - Building also produces the brute-force optimization program `opt`. To
   run, do
       opt input_trace_file
   and it will run the algorithm, printing to `stdout` as it goes. Send
   a SIGINT to stop it and it will print the most optimal solution it
   found so far.
