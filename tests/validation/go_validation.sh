trace=traces/go_trace_aligned.txt
base=tests/validation/go_validation
valid="$base.txt"
res="$base.res"
contents="$base.contents"

./cache_sim 3 2 0 4 2 1 5 2 2 < "$trace" > "$res" 2> "$contents"
diff "$valid" "$res"
