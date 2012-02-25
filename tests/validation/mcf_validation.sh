trace=traces/mcf_trace_aligned.txt
base=tests/validation/mcf_validation
valid="$base.txt"
res="$base.res"
contents="$base.contents"

./cache_sim 9 6 2 10 6 3 11 6 4 < "$trace" > "$res" 2> "$contents"
diff "$valid" "$res"
