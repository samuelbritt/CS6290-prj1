trace=traces/go_trace_aligned.txt
valid=tests/validation/go_validation.txt
res=tests/validation/go_validation.res

./cache_sim 3 2 0 4 2 1 5 2 2 < "$trace" > "$res" 2> "/dev/null"
diff "$valid" "$res"
