cat validation/mcf_validation.txt

echo
echo "--------------"
echo

./cache_sim 9 6 2 10 6 3 11 6 4 < traces/mcf_trace_aligned.txt
