valid=tests/validation/gcc_validation.txt

./cache_sim 9 6 0 10 6 0 11 6 0 < traces/gcc_trace_aligned.txt | diff "$valid" -
./cache_sim 9 6 0 10 6 0 11 6 0 < traces/gcc_trace_aligned.txt
