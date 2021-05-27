/**
 * branchmiss - simple program for testing branch misses.
 * 
 * USAGE: branchmiss [-*]
 *        No additional argument will lead to lots of branchmiss, otherwise
 *        almost no branch misses.
 * 
 * Compile: gcc -O0 branchmiss.c -o branchmiss
 * 
 * The ratio between miss and no miss can be adjusted by macro @MISS_RATIO
 * 
 * Copyright Yicong Yang
 * Author: Yicong Yang <young.yicong@outlook.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#define MISS_RATIO	2

static inline int read_tsc()
{
	unsigned int hi, lo;
#if defined(__x86_64__)
	asm volatile (
		"rdtsc\n\t; movl %%edx, %0\n\t; movl %%eax, %1\n\t"
		: "=r" (hi), "=r" (lo)
		:
		: "%edx", "%eax"
	);
#elif defined(__aarch64__)
	asm volatile (
		"mrs %0, CNTVCT_EL0\n\t"
		:
		: "=r" ((long long)lo)
	);
#endif

	return lo;
}

static inline void nop()
{
	asm volatile (
		"nop\n\t"
		: : :
	);
}

static inline void branch_miss_nop()
{
	nop();
}

static inline void branch_no_miss_nop()
{
	nop();
}

void branch_miss()
{
	int miss = 0;

	while (1)
	{
		miss = read_tsc() % MISS_RATIO;

		if (!miss)
			branch_no_miss_nop();
		else
			branch_miss_nop();
	}
}

void branch_no_miss()
{
	while (1)
		branch_no_miss_nop();
}

int main(int argc, char *argv[])
{
	if (argc > 1)
		branch_no_miss();
	else
		branch_miss();

	return 0;
}
