#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include "../src/cache_sim.h"

int main(int argc, char const *argv[])
{
	/* read entire input into memory */
	FILE *trace_file = fopen(argv[1], "r");
	fseek(trace_file, 0L, SEEK_END);
	size_t filesize = ftell(trace_file);
	rewind(trace_file);
	char *trace_buf = calloc(filesize + 1, 1);
	if (!trace_buf) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
	fread(trace_buf, 1, filesize, trace_file);
	trace_buf[filesize] = '\0';
	fclose(trace_file);

	/* use file i/o on the mem buffer */
	trace_file = fmemopen(trace_buf, filesize, "r");
	/* char output_name[100]; */
	/* sprintf(output_name, "%s.opt", argv[1]); */
	/* FILE *output = fopen(output_name, "w"); */

	struct cache caches[CACHE_COUNT];
	for (int i = 0; i < CACHE_COUNT; ++i) {
		int level = i + 1;
		cache_init(&caches[i], &caches[i+1], level, 0, 0, 0);
	}
	caches[CACHE_COUNT - 1].next = NULL;

	int max_size = 1 << 20;
	int min_size = INT_MAX;
	int min_aat = INT_MAX;
	int min_params[9] = {0};

	/* Yeah. I did that. */
	char rw;
	void *addr;
	int aat;
	int cache_size;
	int c1, b1, s1, c2, b2, s2, c3, b3, s3;
	for (b1 = 5; b1 < 20; b1+=5) {
		for (c1 = b1; c1 < 20; ++c1) {
			for (s1 = 0; s1 < c1 - b1; ++s1) {
				cache_destroy(&caches[0]);
				cache_init(&caches[0], &caches[1], 1, c1, b1, s1);

				for (b2 = b1; b2 < 20; b2+=5) {
					for (c2 = c1; c2 < 20; ++c2) {
						for (s2 = s1; s2 < c2 - b2; ++s2) {
							cache_destroy(&caches[1]);
							cache_init(&caches[1], &caches[2], 2, c2, b2, s2);

							for (b3 = b2; b3 < 20; b3+=5) {
								for (c3 = c2; c3 < 20; c3+=5) {
									for (s3 = s2; s3 < c3 - b3; ++s3) {
										cache_destroy(&caches[2]);
										cache_init(&caches[2], NULL, 3, c3, b3, s3);

										/* cache_destroy(&caches[0]); */
										/* cache_destroy(&caches[1]); */
										/* cache_destroy(&caches[2]); */
										/* cache_init(&caches[0], &caches[1], 1, 9, 6, 0); */
										/* cache_init(&caches[1], &caches[2], 2, 10, 6, 0); */
										/* cache_init(&caches[2], NULL, 3, 11, 6, 0); */

										clearerr(trace_file);
										rewind(trace_file);
										while (!feof(trace_file)) {
											fscanf(trace_file, "%c %p\n", &rw, &addr);
											cache_access(caches, rw2access_type(rw), addr);
										}
										cache_size = total_cache_area(caches);
										if (cache_size > max_size)
											continue;

										aat = average_access_time(caches);
										printf("%d %d %d %d %d %d %d %d %d\n",
											   c1, b1, s1,
											   c2, b2, s2,
											   c3, b3, s3);
										printf("%0.2g, %d\n",
											   cache_size / (float)max_size, aat);

										if (aat < min_aat) {
											min_aat = aat;
											min_size = cache_size;
											min_params[0] = c1;
											min_params[1] = b1;
											min_params[2] = s1;
											min_params[3] = c2;
											min_params[4] = b2;
											min_params[5] = s2;
											min_params[6] = c3;
											min_params[7] = b3;
											min_params[8] = s3;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	printf("Min AAT: %d\n", min_aat);
	printf("Size: %d\n", min_size);
	printf("Utilization: %0.2g\n", min_size / (float) max_size);
	printf("c1: %d\n", min_params[0]);
	printf("b1: %d\n", min_params[1]);
	printf("s1: %d\n", min_params[2]);
	printf("c2: %d\n", min_params[3]);
	printf("b2: %d\n", min_params[4]);
	printf("s2: %d\n", min_params[5]);
	printf("c3: %d\n", min_params[6]);
	printf("b3: %d\n", min_params[7]);
	printf("s3: %d\n", min_params[8]);
}
