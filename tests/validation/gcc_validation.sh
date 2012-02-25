trace=traces/gcc_trace_aligned.txt
base=tests/validation/gcc_validation
valid="$base.txt"
res="$base.res"
contents="$base.contents"

./cache_sim 9 6 0 10 6 0 11 6 0 < "$trace" > "$res" 2> "$contents"
diff "$valid" "$res"
