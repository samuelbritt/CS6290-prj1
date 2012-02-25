#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <signal.h>

#include "../src/cache_sim.h"

volatile sig_atomic_t stop = 0;
void handler(int signo) {
	stop = 1;
}

/*  Description */
static void print_results(const char *input_file, int min_aat, int min_size,
						  float utilization, int *min_params)
{
	printf("\n");
	printf("Results on input %s\n", input_file);
	printf("Min AAT: %d\n", min_aat);
	printf("Size: %d\n", min_size);
	printf("Utilization: %4.02g\n", utilization);
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

/* returns size of file in bytes */
size_t fsize(FILE *fp)
{
	size_t start = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	size_t filesize = ftell(fp);
	fseek(fp, start, SEEK_SET);
	return filesize;
}

int main(int argc, char const *argv[])
{
	/* read entire input into memory */
	const char *trace_filename = argv[1];
	FILE *trace_file = fopen(trace_filename, "r");
	if (!trace_file) {
		fprintf(stderr, "Invalid file\n");
		exit(EXIT_FAILURE);
	}
	size_t filesize = fsize(trace_file);
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

	/* install signal handler to kill this thing */
	struct sigaction sa;
	sa.sa_handler = &handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);

	struct cache caches[CACHE_COUNT];
	for (int i = 0; i < CACHE_COUNT; ++i) {
		int level = i + 1;
		cache_init(&caches[i], &caches[i+1], level, 0, 0, 0);
	}
	caches[CACHE_COUNT - 1].next = NULL;

	int budget = 1 << 20;
	int min_size = INT_MAX;
	float min_utilization = 1.;
	int min_aat = INT_MAX;
	int min_params[9] = {0};

	/* Yeah. I did that. */
	char rw;
	void *addr;
	int aat;
	int cache_size;
	float utilization; // percentage of budget
	int c1, b1, s1, c2, b2, s2, c3, b3, s3;
	for (c1 = 18; c1 >= 0; --c1) {
		for (b1 = c1; b1 >= 0; --b1) {
			for (s1 = 0; s1 <= c1 - b1; s1+=5) {
				cache_destroy(&caches[0]);
				cache_init(&caches[0], &caches[1], 1, c1, b1, s1);

				for (c3 = 19; c3 >= c1; c3-=1) {
					for (b3 = c3; b3 >= 0; b3-=1) {
						for (s3 = c3-b3; s3 > s1; s3-=1) {
							cache_destroy(&caches[2]);
							cache_init(&caches[2], NULL, 3, c3, b3, s3);

							for (c2 = c3; c2 >= c1; --c2) {
								for (b2 = b3; b2 >= b1; b2-=5) {
									for (s2 = s1; s2 <= c2 - b2; s2+=2) {
										cache_destroy(&caches[1]);
										cache_init(&caches[1], &caches[2], 2, c2, b2, s2);

										printf("\n%d %d %d %d %d %d %d %d %d\n",
											   c1, b1, s1,
											   c2, b2, s2,
											   c3, b3, s3);

										cache_size = total_cache_area(caches);
										utilization = cache_size / (float) budget;
										if (cache_size > budget) {
											printf("%4.2g of budget: rejected\n", utilization);
											continue;
										}

										rewind(trace_file);
										while (!feof(trace_file)) {
											fscanf(trace_file, "%c %p\n", &rw, &addr);
											cache_access(caches, rw2access_type(rw), addr);
										}

										aat = average_access_time(caches);
										printf("%4.2g, %d\n", utilization, aat);

										if ((aat < min_aat) ||
											((aat == min_aat) && (cache_size < min_size))) {
											min_aat = aat;
											min_size = cache_size;
											min_utilization = utilization;
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
										if (stop) {
											print_results(trace_filename,
														  min_aat, min_size,
														  min_utilization,
														  min_params);
											return 0;
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
}
