
#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>
#include <assert.h>

//#if USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
//#endif

typedef __int128 int128_t;

int64_t getbit(int64_t value, int n) { return (value >> n) & 1; }
int64_t setbit(int64_t value, int n, int bitValue) { return value & (~(1 << n)) & ((bitValue & 1) << n); }


uint64_t sizeMask(uint64_t x)
{
	uint64_t mask = x;
	mask |= mask >> 1;
	mask |= mask >> 2;
	mask |= mask >> 4;
	mask |= mask >> 8;
	mask |= mask >> 16;
	mask |= mask >> 32;
	return mask;
}

int64_t leastBitGreater(int64_t x)
{
	int64_t mask = sizeMask(x);
	return mask ^ ((mask << 1) & 1);
}

int highestBitIndex(int64_t x)
{
	int64_t i = 63;
	do
	{
		if (x & ((int64_t)1l << i)) { return i; }
		i--;
	} while (i != 0);
	return 0;
}

uint64_t maskBelowLowestZero(uint64_t x)
{
	uint64_t mask = x;
	mask &= (mask <<  1) | 0x1;
	mask &= (mask <<  2) | 0x3;
	mask &= (mask <<  4) | 0xF;
	mask &= (mask <<  8) | 0xFF;
	mask &= (mask << 16) | 0xFFFF;
	mask &= (mask << 32) | 0xFFFFFFFF;
	//printf("maskBelowLowestZero(0x%lx) = 0x%lx\n", x, mask);
	return mask;
}

int64_t inc(int64_t x)
{
	if (~x == 0) { return 0; } // TODO: Carry flag
	int64_t mask = maskBelowLowestZero(x);
	int64_t z1 = mask ^ ((mask << 1) | 1);
	//printf("z1 = 0x%lx\n", z1);
	return (x & ~mask) |
			//(~(x & mask) & mask) |
			z1;
}

int64_t neg(int64_t x)
{
	return inc(~x);
}

int64_t maskAboveLowestOne(int64_t x)
{
	int64_t m = x;
	m |= m << 1;
	m |= m << 2;
	m |= m << 4;
	m |= m << 8;
	m |= m << 16;
	m |= m << 32;
	//printf("maskAboveLowestOne(0x%lx) = 0x%lx\n", x, m);
	return m;
}

int64_t dec(int64_t x)
{
	if (x == 0) { return 0xFFFFFFFFFFFFFFFF; } // TODO: Carry flag
	int64_t mask = maskAboveLowestOne(x) << 1;
	//int64_t z1 = mask ^ ((mask << 1) | 1);
	return (x & mask) | ((~x) & (~mask));
}

int64_t imul32(int32_t a, int32_t b)
{
	int64_t r = 0;
	int64_t a64 = (int64_t)a;
	int64_t b64 = (int64_t)b;
	for (int i = 0; i < 32; i++)
	{
		if (getbit(b64, i))
		{
			r += a64 << i;
		}
	}
	return r;
}

int128_t imul64(int64_t a, int64_t b)
{
	int128_t r = 0;
	int128_t a128 = a;
	int128_t b128 = b;
	for (int i = 0; i < 128; i++)
	{
		if (b128 & ((int128_t)1 << i))
		{
			r += a128 << i;
		}
	}
	return r;
}



int64_t xx_idiv32(int32_t a, int32_t b)
{
	int hba = highestBitIndex(a);
	int hbb = highestBitIndex(b);
	int shift = hba - hbb + 32;
	int64_t a64 = (int64_t)a << 32;
	int64_t b64 = (int64_t)b; // << 32;
	int64_t r = a64;
	int64_t q = 0;
	for (int i = shift; i >=0; i--)
	{
		int64_t diff = r - (b64 << i);
		if (diff > 0)
		{
			q |= 1l << i;
			r = diff;
		}
		//if (getbit(a64, i))
		//r -= b64 >> i;
	}
	return r;
}

typedef struct
{
	int32_t q;
	int32_t r;
} divmod_result_32;

typedef struct
{
	int64_t q;
	int64_t r;
} divmod_result_64;

divmod_result_32 idivmod32(int32_t a, int32_t b)
{
	int a_is_negative = (a >> 31) & 1;
	int b_is_negative = (b >> 31) & 1;
	//int negative = (a >> 31) ^ (b >> 31);

	//if (a_is_negative) { printf("a is negative\n"); }
	//if (b_is_negative) { printf("b is negative\n"); }

	int negative = a_is_negative ^ b_is_negative;
	if (a_is_negative) { a = neg(a); } //~a + 1; }
	if (b_is_negative) { b = neg(b); } // ~b + 1; }

	int hba = highestBitIndex(a);
	int hbb = highestBitIndex(b);
	int shift = hba - hbb;
	int32_t r = a;
	int32_t q = 0;
	for (int i = shift; i >=0; i--)
	//for (int i = 32 - hbb; i >=0; i--)
	{
		int32_t diff = r - (b << i);
		if (diff >= 0)
		{
			q |= 1l << i;
			r = diff;
		}
	}
	if (negative)
	//if (a_is_negative || b_is_negative)
	//if (b_is_negative)
	{
		//q = ~q + 1;
		//r = ~r + 1;
		printf("b_is_negative\n");
		q = neg(q);
	}
	if (a_is_negative) {
		printf("a_is_negative\n");
		r = neg(r);
	}
	//printf("idiv32(%d, %d): %d  rem %d\n", a, b, q, r);
	//return ((int64_t)q << 32) | (r & 0xFFFFFFFF);
	divmod_result_32 ret = { .q=q, .r=r };
	return ret;
}

divmod_result_64 idivmod64(int64_t a, int64_t b)
{
	int a_is_negative = (a >> 63) & 1;
	int b_is_negative = (b >> 63) & 1;
	//int negative = (a >> 31) ^ (b >> 31);

	//if (a_is_negative) { printf("a is negative\n"); }
	//if (b_is_negative) { printf("b is negative\n"); }

	int negative = a_is_negative ^ b_is_negative;
	int64_t absa, absb;

	absa = a_is_negative ? neg(a) : a;
	absb = b_is_negative ? neg(b) : b;

	int hba = highestBitIndex(absa);
	int hbb = highestBitIndex(absb);
	int shift = hba - hbb;
	int64_t r = absa;
	int64_t q = 0;
	for (int i = shift; i >=0; i--)
		//for (int i = 32 - hbb; i >=0; i--)
	{
		int64_t diff = r - (absb << i);
		if (diff >= 0)
		{
			q |= (int64_t)1 << i;
			r = diff;
		}
	}
	if (negative)
		//if (a_is_negative || b_is_negative)
		//if (b_is_negative)
	{
		//q = ~q + 1;
		//r = ~r + 1;
		//printf("b_is_negative\n");
		q = neg(q);
	}
	if (a_is_negative) {
		//printf("a_is_negative\n");
		r = neg(r);
	}
	//printf("idiv32(%d, %d): %d  rem %d\n", a, b, q, r);
	//return ((int64_t)q << 32) | (r & 0xFFFFFFFF);
	divmod_result_64 ret = { .q=q, .r=r };
	return ret;
}


float fmul32(float a, float b)
{
	// TODO: Special values like NaN and Inf
	// integer representation
	int32_t ia = *(int32_t*)&a;
	int32_t ib = *(int32_t*)&b;
	// sign
	int sa = ia >> 31;
	int sb = ib >> 31;
	// mantissa
	int32_t ma = (ia & 0x7FFFFF) | 0x800000;
	int32_t mb = (ib & 0x7FFFFF) | 0x800000;
	// exponent
	int32_t ea = ((ia >> 23) & 0xFF);
	int32_t eb = ((ib >> 23) & 0xFF);
	// 'r' = result
	int64_t mrtmp = imul32(ma, mb) >> 23;
	int mshift = getbit(mrtmp, 24);
	//printf("mshift = %d\n", mshift);
	int64_t mr = mrtmp >> mshift;
	int32_t ertmp = ea + eb - 127;
	int32_t er = mshift ? inc(ertmp) : ertmp;
	// TODO: Overflow ^
	int sr = sa ^ sb;
	int32_t r = (sr << 31) | ((er & 0xFF) << 23) | (mr & 0x7FFFFF);
	return *(float*)&r;
}

float fdiv32(float a, float b)
{
	// integer representation
	int32_t ia = *(int32_t*)&a;
	int32_t ib = *(int32_t*)&b;
	// sign
	int sa = ia >> 31;
	int sb = ib >> 31;
	// mantissa
	int32_t ma = (ia & 0x7FFFFF) | 0x800000;
	int32_t mb = (ib & 0x7FFFFF) | 0x800000;
	// exponent
	int32_t ea = ((ia >> 23) & 0xFF);
	int32_t eb = ((ib >> 23) & 0xFF);

	// Much like doing fixed point division, shift the dividend left
	// past the decimal point first.
	divmod_result_64 dr = idivmod64((int64_t)ma << 23, mb);
	int mshift = 23 - highestBitIndex(dr.q);
	int32_t mr = (dr.q << mshift);
	int32_t er = ea - eb - mshift + 127;
	int32_t sr = sa ^ sb;
	int32_t r = (sr << 31) | ((er & 0xFF) << 23) | (mr & 0x7FFFFF);
	return *(float*)&r;
}


typedef enum
{
	INPUT_INT32,
	INPUT_INT64,
	INPUT_FLOAT32,
	INPUT_FLOAT64
} INPUT_TYPE;

typedef enum
{
	OP_INC,
	OP_DEC,
	OP_IMUL32,
	OP_IDIV32,
	OP_FMUL32,
	OP_FDIV32,
	OP_IMUL64,
	OP_IDIV64,
} OPERATION;


void printDecimalInt128(int128_t n)
{
	char buf[100];
	int i = 0;
	int neg = 0;
	if (n & ((int128_t)1 << 127)) { neg = 1; }
	while (n != 0) // ((int64_t)0xFFFFFFFFFFFFFFFF))
	{
		// If this was part of the bit math logic, I would use neg() and sign bit detection,
		// but this is just for the sake of printing so I'm going to use the
		// standard library.
		int128_t q = n / 10;
		int r = abs((int)(n % 10));
		buf[i++] = '0' + r;
		n = q;
	}
	if (neg) { putc('-', stdout); }
	while (--i >= 0)
	{
		putc(buf[i], stdout);
	}
}

int runOpLoop(INPUT_TYPE mode, int numInputs, OPERATION op)
{
	float f32a, f32b, f32r;
	double f64a, f64b, f64r;
	int32_t i32a, i32b, i32r;
	int64_t i64a, i64b, i64r;

	char* line;
	//char* line_b;
	printf("%s", "Enter an empty line to quit.\n");
	int quit = 0;
	while (1)
	{
		line = readline("a = ");
		if (strlen(line) == 0) { quit = 1; }
		else
		{
			add_history(line);
			switch (mode)
			{
			case INPUT_INT32:
				//i32a = strtoi(line, NULL, 10, INT32_MIN, INT32_MAX, NULL);
				i32a = atoi(line);
				break;
			case INPUT_INT64:
				//i64a = strtoi(line, NULL, 10, INT64_MIN, INT64_MAX, NULL);
				i64a = atol(line);
				break;
			case INPUT_FLOAT32:
				//f32a = strtof(line, NULL);
				f32a = atof(line);
				break;
			case INPUT_FLOAT64:
				//f64a = strtod(line, NULL);
				f64a = atof(line);
				break;
			default:
				assert("bad input mode");
			}
		}
		//a = strtof(line_a, NULL, 10, INT64_MIN, INT64_MAX, NULL);
		free(line);
		if (quit) break;

		if (numInputs > 1)
		{
			line = readline("b = ");
			if (strlen(line) == 0) { quit = 1; }
			else
			{
				add_history(line);
				switch (mode)
				{
				case INPUT_INT32:
					//i32b = strtoi(line, NULL, 10, INT32_MIN, INT32_MAX, NULL);
					i32b = atoi(line);
					break;
				case INPUT_INT64:
					//i64b = strtoi(line, NULL, 10, INT64_MIN, INT64_MAX, NULL);
					i64b = atol(line);
					break;
				case INPUT_FLOAT32:
					//f32b = strtof(line, NULL);
					f32b = atof(line);
					break;
				case INPUT_FLOAT64:
					//f64b = strtod(line, NULL);
					f64b = atof(line);
					break;
				default:
					assert("bad input mode");
				}
			}
			free(line);
			if (quit) { break; }
		}

		divmod_result_32 dmr32;
		divmod_result_64 dmr64;
		int128_t r128;
		switch (op)
		{
		case OP_INC:
			i64r = inc(i64a);
			printf("%ld ++ = %ld\n", i64a, i64r);
			break;
		case OP_DEC:
			i64r = dec(i64a);
			printf("%ld -- = %ld\n", i64a, i64r);
			break;
		case OP_IMUL32:
			i64r = imul32(i32a, i32b);
			printf("%d * %d = %ld\n", i32a, i32b, i64r);
			break;
		case OP_IDIV32:
			dmr32 = idivmod32(i32a, i32b);
			printf("%d / %d = %d R %d\n", i32a, i32b, dmr32.q, dmr32.r);
			break;
		case OP_FMUL32:
			f32r = fmul32(f32a, f32b);
			printf("%g * %g = %g\n", f32a, f32b, f32r);
			break;
		case OP_FDIV32:
			f32r = fdiv32(f32a, f32b);
			printf("%g / %g = %g\n", f32a, f32b, f32r);
			break;
		case OP_IDIV64:
			dmr64 = idivmod64(i64a, i64b);
			printf("%ld / %ld = %ld R %ld\n", i64a, i64b, dmr64.q, dmr64.r);
			break;
		case OP_IMUL64:
			r128 = imul64(i64a, i64b);
			printf("%ld * %ld = ", i64a, i64b);
			printDecimalInt128(r128);
			printf("\n");
			break;
		default:
			assert("Bad operation selection");
		}
	}
	return 0;
}

int main()
{
	int choice = 0;
	int quit = 0;
	char* line;
	do
	{
		printf("%s", " ==== BIT MATH DEMO ==== \n");
		printf("%s", "1. int64 increment\n");
		printf("%s", "2. int64 decrement\n");
		printf("%s", "3. int32 multiply\n");
		printf("%s", "4. int32 divide\n");
		printf("%s", "5. float32 multiply\n");
		printf("%s", "6. float32 divide\n");
		printf("%s", "7. int64 multiply\n");
		printf("%s", "8. int64 divide\n");
		printf("%s", "0 / nothing: Quit.\n");

		line = readline("Choose: ");
		if (strlen(line) == 0) { quit = 1; }
		//choice = strtoi(line, NULL, 10, INT32_MIN, INT32_MAX, NULL);
		choice = atoi(line); //, NULL, 10, INT32_MIN, INT32_MAX, NULL);
		free(line);
		if (quit) { break; }

		switch (choice)
		{
		case 0:
			break;
		case 1:
			runOpLoop(INPUT_INT64, 1, OP_INC);
			break;
		case 2:
			runOpLoop(INPUT_INT64, 1, OP_DEC);
			break;
		case 3:
			runOpLoop(INPUT_INT32, 2, OP_IMUL32);
			break;
		case 4:
			runOpLoop(INPUT_INT32, 2, OP_IDIV32);
			break;
		case 5:
			runOpLoop(INPUT_FLOAT32, 2, OP_FMUL32);
			break;
		case 6:
			runOpLoop(INPUT_FLOAT32, 2, OP_FDIV32);
			break;
		case 7:
			runOpLoop(INPUT_INT64, 2, OP_IMUL64);
			break;
		case 8:
			runOpLoop(INPUT_INT64, 2, OP_IDIV64);
			break;
		default:
			printf("%s", "That option is not implemented, sorry.\n");
			//continue;
		}
	} while (choice != 0);
	return 0;
}
