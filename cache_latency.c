#include <assert.h>
#include <getopt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>

#define RANDOM_STRIDE_SZ	10
static long int random_stride[RANDOM_STRIDE_SZ] = { 1 };
static const int interations = 100;

long int delta_ns(struct timespec *begin, struct timespec *end)
{
	return (end->tv_sec - begin->tv_sec) * 1000000000 +
		end->tv_nsec - begin->tv_nsec;
}

void init_random_stride(size_t list_len)
{
	srandom(clock());
	for (int i = 0; i < RANDOM_STRIDE_SZ; i++)
		random_stride[i] = random() % list_len;
}

void *alloc_buffer(size_t len, bool tph)
{
	void *buf = malloc(len);

	if (tph)
		madvise(buf, len, MADV_HUGEPAGE);

	return buf;
}

void free_buffer(void *buffer, size_t len)
{
	free(buffer);
}

void init_circular_buffer(void *buf, size_t len)
{
	struct timespec begin, end;
	size_t next, pos = 0, list_len, res;
	int64_t *base = buf;

	list_len = len / sizeof(int64_t);
	res = list_len - 1; /* Init list_len - 1 elems */

	memset(base, -1, len);

	clock_gettime(CLOCK_MONOTONIC, &begin);
	while (res--) {
		next = (pos + random_stride[pos % RANDOM_STRIDE_SZ]) % list_len;
		while (next == pos || next == 0 || base[next] != -1)
			next = (next + 1) % list_len;

		base[pos] = next;
		pos = base[pos];
	}

	assert(base[pos] == -1);
	base[pos] = 0;
	clock_gettime(CLOCK_MONOTONIC, &end);

	printf("init end using %.06f sec\n", (float)delta_ns(&begin, &end) / 1000000000);
}

unsigned long int __attribute__ ((optimize("O0"))) overhead_of_list_walk(int iter, int count)
{
	struct timespec begin, end;
	clock_gettime(CLOCK_MONOTONIC, &begin);

	while (iter--)
		for (int i = 0; i < count; i++)
			;

	clock_gettime(CLOCK_MONOTONIC, &end);

	return delta_ns(&begin, &end);
}

void walk_circular_list(void *buf, size_t len)
{
	unsigned long int overhead, total, real;
	struct timespec begin, end;
	volatile size_t dummy;
	int64_t *base = buf;
	int count = 1000, iter = interations;
	size_t pos = 0;

	overhead = overhead_of_list_walk(iter, count);

	clock_gettime(CLOCK_MONOTONIC, &begin);
	while (iter--) {
		for (int i = 0; i < count; i++) {
			pos = base[pos];
			pos = base[pos];
			pos = base[pos];
			pos = base[pos];
			pos = base[pos];
			pos = base[pos];
			pos = base[pos];
			pos = base[pos];
			pos = base[pos];
			pos = base[pos];
			pos = base[pos];
			pos = base[pos];
			pos = base[pos];
			pos = base[pos];
			pos = base[pos];
			pos = base[pos];
		}
	}
	dummy = pos;
	clock_gettime(CLOCK_MONOTONIC, &end);

	total = delta_ns(&begin, &end);
	real = total - overhead;

	printf("time cost: %ld ns, overhead %ld ns, total %ld ns\n", real, overhead, total);
	printf("Per load %.06f ns\n", (float)real / (16 * count * interations + 1));
}

int main(int argc, char *argv[])
{
	struct option opts[] = {
		{ "length",   required_argument, NULL, 'l' },
		{ "hugepage", no_argument,       NULL, 'h' },
	};
	size_t length = 32 * 1024 * 1024;
	bool thp = false;
	int c, opt_index;
	void *buffer;

	while ((c = getopt_long(argc, argv, "l:h", opts, &opt_index)) != EOF) {
		switch (c)
		{
		case 'l':
			length = strtoul(optarg, NULL, 0);
			break;
		case 'h':
			thp = true;
		default:
			break;
		}
	}
	
	buffer = alloc_buffer(length, thp);

	init_random_stride(length / sizeof(int64_t));

	init_circular_buffer(buffer, length);

	walk_circular_list(buffer, length);

	free_buffer(buffer, length);
	return 0;
}



