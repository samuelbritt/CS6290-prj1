trace=traces/gcc_trace_aligned.txt
valid=tests/validation/gcc_validation.txt
res=tests/validation/gcc_validation.res

./cache_sim 9 6 0 10 6 0 11 6 0 < "$trace" > "$res" 2> "/dev/null"
diff "$valid" "$res"
