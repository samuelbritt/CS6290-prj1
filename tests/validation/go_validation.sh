valid=tests/validation/go_validation.txt

./cache_sim 3 2 0 4 2 1 5 2 2 < traces/go_trace_aligned.txt | diff "$valid" -
