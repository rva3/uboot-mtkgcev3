#include <linux/types.h>

typedef union {
	uint64_t all;
	struct {
		uint32_t lo;
		uint32_t hi;
	} s;
} dw_t;

uint64_t __lshrdi3(uint64_t u, unsigned int b)
{
	dw_t in, out;

	in.all = u;
	if (b >= 64)
		return 0;
	if (b >= 32) {
		out.s.lo = in.s.hi >> (b - 32);
		out.s.hi = 0;
	} else if (b == 0) {
		return u;
	} else {
		out.s.hi = in.s.hi >> b;
		out.s.lo = (in.s.hi << (32 - b)) | (in.s.lo >> b);
	}
	return out.all;
}

uint64_t __ashldi3(uint64_t u, unsigned int b)
{
	dw_t in, out;

	in.all = u;
	if (b >= 64)
		return 0;
	if (b >= 32) {
		out.s.hi = in.s.lo << (b - 32);
		out.s.lo = 0;
	} else if (b == 0) {
		return u;
	} else {
		out.s.hi = (in.s.hi << b) | (in.s.lo >> (32 - b));
		out.s.lo = in.s.lo << b;
	}
	return out.all;
}

int64_t __ashrdi3(int64_t u, unsigned int b)
{
	dw_t in, out;
	int32_t hi;

	in.all = (uint64_t)u;
	hi = (int32_t)in.s.hi;
	if (b >= 64)
		return (hi < 0) ? -1 : 0;
	if (b >= 32) {
		out.s.lo = (uint32_t)(hi >> (b - 32));
		out.s.hi = (hi < 0) ? 0xffffffffU : 0;
	} else if (b == 0) {
		return u;
	} else {
		out.s.lo = (in.s.lo >> b) | ((uint32_t)hi << (32 - b));
		out.s.hi = (uint32_t)(hi >> b);
	}
	return (int64_t)out.all;
}

uint64_t __muldi3(uint64_t a, uint64_t b)
{
	uint32_t al = (uint32_t)a, ah = (uint32_t)(a >> 32);
	uint32_t bl = (uint32_t)b, bh = (uint32_t)(b >> 32);
	uint32_t lo = al * bl;
	uint32_t mid_lo = ah * bl + al * bh;
	uint32_t carry1 = (mid_lo < ah * bl);
	uint32_t hi = ah * bh + (mid_lo >> 16) + (carry1 << 16);
	uint32_t t = (lo >> 16) + (mid_lo & 0xffff);

	hi += t >> 16;
	return ((uint64_t)hi << 32) |
	       ((lo & 0xffff) | ((mid_lo & 0xffff) << 16));
}

static uint64_t __udivmoddi3(uint64_t num, uint64_t den, uint64_t *rem)
{
	uint64_t q = 0, r = 0;
	int i;

	if (den == 0) {
		if (rem)
			*rem = 0;
		return 0;
	}
	for (i = 63; i >= 0; i--) {
		uint32_t bit = (i >= 32) ?
				       (uint32_t)(num >> 32) >> (i - 32) & 1U :
				       (uint32_t)num >> i & 1U;

		r = (r << 1) | bit;
		if (r >= den) {
			r -= den;
			q |= (uint64_t)1 << i;
		}
	}
	if (rem)
		*rem = r;
	return q;
}

uint64_t __udivdi3(uint64_t a, uint64_t b)
{
	return __udivmoddi3(a, b, NULL);
}

uint64_t __umoddi3(uint64_t a, uint64_t b)
{
	uint64_t r;

	__udivmoddi3(a, b, &r);
	return r;
}

int64_t __divdi3(int64_t a, int64_t b)
{
	int neg = (a < 0) ^ (b < 0);
	uint64_t ua = (a < 0) ? (uint64_t)-(a + 1) + 1 : (uint64_t)a;
	uint64_t ub = (b < 0) ? (uint64_t)-(b + 1) + 1 : (uint64_t)b;
	uint64_t q = __udivmoddi3(ua, ub, NULL);

	return neg ? -(int64_t)q : (int64_t)q;
}

int64_t __moddi3(int64_t a, int64_t b)
{
	int neg = (a < 0);
	uint64_t ua = (a < 0) ? (uint64_t)-(a + 1) + 1 : (uint64_t)a;
	uint64_t ub = (b < 0) ? (uint64_t)-(b + 1) + 1 : (uint64_t)b;
	uint64_t r;

	__udivmoddi3(ua, ub, &r);
	return neg ? -(int64_t)r : (int64_t)r;
}

int __clzsi2(unsigned int x)
{
	int n = 0;

	if (!(x >> 16)) {
		n += 16;
		x <<= 16;
	}
	if (!(x >> 24)) {
		n += 8;
		x <<= 8;
	}
	if (!(x >> 28)) {
		n += 4;
		x <<= 4;
	}
	if (!(x >> 30)) {
		n += 2;
		x <<= 2;
	}
	if (!(x >> 31)) {
		n += 1;
	}
	return x ? n : 32;
}

int __clzdi2(uint64_t x)
{
	uint32_t hi = (uint32_t)(x >> 32);

	if (hi)
		return __clzsi2(hi);
	return __clzsi2((uint32_t)x) + 32;
}

int __ctzsi2(unsigned int x)
{
	int n = 0;

	if (!x)
		return 32;
	if (!(x << 16)) {
		n += 16;
		x >>= 16;
	}
	if (!(x << 24)) {
		n += 8;
		x >>= 8;
	}
	if (!(x << 28)) {
		n += 4;
		x >>= 4;
	}
	if (!(x << 30)) {
		n += 2;
		x >>= 2;
	}
	if (!(x << 31)) {
		n += 1;
	}
	return n;
}

int __ctzdi2(uint64_t x)
{
	uint32_t lo = (uint32_t)x;

	if (lo)
		return __ctzsi2(lo);
	return __ctzsi2((uint32_t)(x >> 32)) + 32;
}
