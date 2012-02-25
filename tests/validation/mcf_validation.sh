trace=traces/mcf_trace_aligned.txt
valid=tests/validation/mcf_validation.txt
res=tests/validation/mcf_validation.res

./cache_sim 9 6 2 10 6 3 11 6 4 < "$trace" > "$res" 2> "/dev/null"
diff "$valid" "$res"
