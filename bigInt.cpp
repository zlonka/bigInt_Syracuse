// bigint32.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#define  _CRT_SECURE_NO_WARNINGS 1


#define _CRT_RAND_S  
#include <iostream>
#include <time.h>
#include <stdlib.h>	// for random

// #define _CRT_SECURE_NO_WARNINGS
#define SIZ 32
#if SIZ < 64
#define BIGMAX        100000000
#define BIGMAX_SUR_2  50000000
#define BIGMAX_SUR_10 10000000
#define BIGMAX_NBDIGITS 8
#define uint unsigned int
#define FMT0 "%u"
#define FMT1 "%08u"
#define FMT1multi "%08u,%08u,...,%08u,%08u"
#define NB 8
#else
#define BIGMAX 10000000000000000LL
#define uint unsigned long long
#define FMT0 "%llu"
#define FMT1 "%16llu"
#define FMT1multi "%16llu,%16llu,...,%16llu,%16llu"
#define NB 16
#endif
typedef struct {
	uint n, nAlloc;
	uint* t;
}BIG;

static const bool dbg = true;

static const int BIG_memInit = 4;
static const int BIG_memStep = 1024;
void big_init(BIG* a, const uint v = 0, const uint nbAlloc = BIG_memInit);
void big_dump(BIG* a, const char* str);
void big_set(BIG* a, const uint v);
void big_init(BIG* a, const uint v, const uint nbAlloc) {
	a->nAlloc = nbAlloc;
	a->t = (uint*)malloc(a->nAlloc * sizeof(uint));
	a->t[0] = v;
	a->n = 1;
}
void big_shiftUp(BIG* a) {
	for (uint i = a->n - 1; i > 0; i--) a->t[i] = a->t[i - 1];
	// leaves t[0] unchanged (now equals to t[1])
}
void big_shiftDown(BIG* a) {
	for (uint i = 0; i < a->n - 1; i++) a->t[i] = a->t[i + 1];
	a->n--;
	if (a->n == 0) {
		a->n = 1; a->t[0] = 0;
	}
}
bool big_setStr(BIG* a, const char* str) {
	// check valid str
	int l = (int)strlen(str);
	if (l == 0) return false;
	for (int i = 0; i < l; i++) if (str[i] > '9' || str[i] < '0') return false;
	// check for leading zeroes
	int i = 0; while (i < l - 1 && str[i] == '0') i++;
	l -= i;
	// if (l == 0) { a->n = 1; a->t[0] = 0; return false; }
	char* strLocal = (char*)str + i;
	printf("big_setStr::l=%d\n", l);
	if (l > BIGMAX_NBDIGITS) {
		uint nb = (l + (BIGMAX_NBDIGITS - 1)) / BIGMAX_NBDIGITS;
		// printf("big_setStr::nb=%d\n", nb);
		if (a->nAlloc < nb) {
			a->n = a->nAlloc = nb;
			a->t = (uint*)realloc(a->t, a->nAlloc * sizeof(uint));
		}
		a->n = nb;
		char word[BIGMAX_NBDIGITS + 2];
		for (int i = 0; i < BIGMAX_NBDIGITS + 1; i++) word[i] = '\0';
		// first
		int x = nb - 1, nbResidu = l % BIGMAX_NBDIGITS;
		if (nbResidu) {
			memcpy(word, strLocal, nbResidu);
			sscanf(word, "%u", &a->t[x]);
			// printf("big_setStr::x=%d word='%s'\n", x, word);
			// printf("big_setStr::nbResidu=%d\n", nbResidu);
			// big_dump(a, "\n");
			x--;
		}
		// next ones
		for (int i = nbResidu; i < l; i += BIGMAX_NBDIGITS, x--) {
			memcpy(word, &strLocal[i], BIGMAX_NBDIGITS);
			sscanf(word, "%u", &a->t[x]);
			// printf("big_setStr::i=%d x=%d word='%s'\n", i, x, word);
			// big_dump(a, "\n");
		}
	}
	else {
		sscanf(strLocal, "%u", &a->t[0]);
		a->n = 1;
	}
	return true;
}
void big_set(BIG* a, const uint v) {
	if (v >= BIGMAX) {
		a->t[0] = v % BIGMAX;
		a->t[1] = v / BIGMAX;
		a->n = 2;
	}
	else {
		a->t[0] = v;
		a->n = 1;
	}
}
void big_copy(BIG* dst, BIG* src) {
	if (dst->nAlloc < src->nAlloc) {
		dst->nAlloc = src->nAlloc;
		dst->t = (uint*)realloc(dst->t, dst->nAlloc * sizeof(uint));
	}
	memcpy(dst->t, src->t, src->n * sizeof(uint));
	dst->n = src->n;
}
void big_free(BIG* a) {
	if (a->t) {
		free(a->t);
		a->t = (uint*)NULL;
		a->n = 0;
		a->nAlloc = 0;
	}
}
void big_swap(BIG* a, BIG* b) {
	uint tmp = a->n;
	a->n = b->n; b->n = tmp;
	tmp = a->nAlloc; a->nAlloc = b->nAlloc; b->nAlloc = tmp;
	uint* ptr = a->t;
	a->t = b->t; b->t = ptr;
}
void big_mul(BIG* c, BIG* a, BIG* b) {   // c=a*b
	if ((a->n == 1 && a->t[0] == 0) || (b->n == 1 && b->t[0] == 0)) {   // mul by 0
		c->n = 0; c->t[0] = 0; return;
	}
	if (a->n == 1 && a->t[0] == 1) {   // a=1 : return b
		big_copy(c, b); return;
	}
	if (b->n == 1 && b->t[0] == 1) {   // b=1 : return a
		big_copy(c, a); return;
	}
	if (c->nAlloc < a->nAlloc + b->nAlloc + 2) {
		c->nAlloc = a->nAlloc + b->nAlloc + 2;
		c->t = (uint*)realloc(c->t, c->nAlloc * sizeof(uint));
	}
	c->n = a->n + b->n + 2;
	for (uint i = 0; i < c->n; i++) c->t[i] = 0;
	// printf("init c done.\n");
	// printf("c->n=%d\n", c->n);

	for (uint i = 0; i < a->n; i++) {
		// printf("i=%d/%d\n", i, a->n);
		for (uint j = 0; j < b->n; j++) {
			// printf("j=%d/%d\n", j, b->n);
			unsigned __int64 tmp = (unsigned __int64)a->t[i] * (unsigned __int64)b->t[j];
			// printf("tmp = %u * %u = %I64u\n", a->t[i], b->t[j], tmp);
			int low = tmp % BIGMAX, hi = tmp / BIGMAX;
			// printf("low=%d hi=%d\n", low, hi);
			c->t[i + j] += low;
			// printf("c->t[i+j]=%d\n", c->t[i + j]);
			if (c->t[i + j] >= BIGMAX) {
				c->t[i + j] -= BIGMAX;
				c->t[i + j + 1] ++;
				// printf("c->t[i+j+1]=%d\n", c->t[i + j + 1]);
			}
			c->t[i + j + 1] += hi;
			if (c->t[i + j + 1] >= BIGMAX) {
				c->t[i + j + 1] -= BIGMAX;
				c->t[i + j + 2] ++;
				// printf("c->t[i+j+2]=%d\n", c->t[i + j + 2]);
			}
		}
	}
	// remove leading 0s
	int i = c->n - 1;
	while (i > 0 && c->t[i] == 0) {
		i--;
	}
	c->n = i + 1;
	// printf("c->n=%d c=", c->n);
	// big_dump(c, "\n");
}
void big_pow(BIG* dest, BIG* a, const uint p) {   // dest=a^p
	BIG c;
	big_set(dest, 1);
	if (p == 0) return;
	big_copy(dest, a);
	big_init(&c);
	big_copy(&c, a);
	for (uint i = 1; i < p; i++) {
		big_mul(dest, &c, a);
		// big_printf(&a, " * ");
		// big_printf(&b, " = ");
		// big_printf(&c, "\n");
		big_copy(&c, dest);
	}
	big_free(&c);
}
void big_mulDigit(BIG* a, const uint v) {   // a*=v
	uint carry = 0;
	if (v == 0) {
		a->t[0] = 0; a->n = 1; return;
	}
	if (v == 1) {
		return;
	}
	for (uint i = 0; i < a->n; i++) {
		a->t[i] = a->t[i] * v + carry;
		if (a->t[i] >= BIGMAX) {
			carry = a->t[i] / BIGMAX;
			a->t[i] %= BIGMAX;
		}
		else carry = 0;
	}
	if (carry) {
		if (a->nAlloc <= a->n) {
			a->nAlloc += BIG_memStep;
			a->t = (uint*)realloc(a->t, a->nAlloc * sizeof(uint));
		}
		a->t[a->n] = carry;
		a->n++;
	}
}
void big_inc(BIG* a) {   // a++
	uint carry = 0;
	for (uint i = 0; i < a->n; i++) {
		a->t[i] ++;
		if (a->t[i] >= BIGMAX) {
			carry = 1;
			a->t[i] -= BIGMAX;
		}
		else {
			return;
			// carry = 0;
			// break;
		}
	}
	if (carry) {
		if (a->nAlloc <= a->n) {
			a->nAlloc += BIG_memStep;
			a->t = (uint*)realloc(a->t, a->nAlloc * sizeof(uint));
		}
		a->t[a->n] = carry;
		a->n++;
	}
}
void big_mulDigitBy2(BIG* a) {  // a *= 2;
	uint carry = 0;
	for (uint i = 0; i < a->n; i++) {
		a->t[i] += a->t[i];
		a->t[i] += carry;
		if (a->t[i] >= BIGMAX) {
			carry = 1;
			a->t[i] -= BIGMAX;
		}
		else carry = 0;
	}
	if (carry) {
		if (a->nAlloc <= a->n) {
			a->nAlloc += BIG_memStep;
			a->t = (uint*)realloc(a->t, a->nAlloc * sizeof(uint));
		}
		a->t[a->n] = carry;
		a->n++;
	}
}
void big_divDigitBy2(BIG* a) {  // a /= 2;
	// 1st no test
	a->t[0] >>= 1;
	for (uint i = 1; i < a->n; i++) {
		if (a->t[i] & 1) a->t[i - 1] += BIGMAX_SUR_2;
		a->t[i] >>= 1;
		// big_dump(a, "\n");
		// big_dump(a, "\n----\n");
	}
	if (a->n > 1 && a->t[a->n - 1] == 0) a->n--;
}
bool big_gt(BIG* a, BIG* b) {
	if (a->n > b->n) return true;
	else if (a->n < b->n) return false;
	for (int i = (int)a->n - 1; i >= 0; i--) if (a->t[i] > b->t[i]) return true; else if (a->t[i] < b->t[i]) return false;
	return false;
}
bool big_gte(BIG* a, BIG* b) {
	if (a->n > b->n) return true;
	else if (a->n < b->n) return false;
	for (int i = (int)a->n - 1; i >= 0; i--) if (a->t[i] < b->t[i]) return false;
	return true;
}
bool big_eq(BIG* a, BIG* b) {
	if (a->n != b->n) return false;
	for (int i = (int)a->n - 1; i >= 0; i--) if (a->t[i] != b->t[i]) return false;
	return true;
}
void big_printf(BIG* a, const char* str = NULL) {
	printf("%d", a->t[a->n - 1]);
	for (int i = a->n - 2; i >= 0; i--) printf(FMT1, a->t[i]);
	if (str != NULL) printf("%s", str);
}
void big_fprintf(FILE* f, BIG* a, const char* str = NULL) {
	fprintf(f, "%d", a->t[a->n - 1]);
	for (int i = a->n - 2; i >= 0; i--) fprintf(f, FMT1, a->t[i]);
	if (str != NULL) fprintf(f, "%s", str);
}
void big_dump(BIG* a, const char* str = NULL) {
	printf("{n=%u, nAlloc=%u, t=[", a->n, a->nAlloc);
	if (a->n > 4) {
		printf(FMT1multi, a->t[0], a->t[1], a->t[a->n - 2], a->t[a->n - 1]);
	}
	else {
		for (uint i = 0; i < a->n; i++) {
			if (i != 0) printf(",");
			printf(FMT1, a->t[i]);
		}
	}
	printf("]}");
	if (str != NULL) printf("%s", str);
}
void big_niceprintf(BIG* a, const char* str = NULL) {
	char* buf;
	buf = (char*)malloc((1 + NB * a->n) * sizeof(char));
	sprintf(buf, FMT0, a->t[a->n - 1]);
	for (int i = a->n - 2; i >= 0; i--) {
		char small[16];
		sprintf(small, FMT1, a->t[i]);
		strcat(buf, small);
	}
	int len = (int)strlen(buf);
	for (int i = 0, j = 3 - (len + 2) % 3;; j++) {
		printf("%c", buf[i++]);
		if (buf[i] == 0) break;
		if (j == 3) {
			j = 0;
			printf(".");
		}
	}
	free(buf);
	if (str != NULL) printf("%s", str);
}
void big_muldigits(BIG* a) {
	BIG b;
	big_init(&b, 1);
	for (int i = a->n - 1; i >= 0; i--) {
		uint v = a->t[i];
		while (v) {
			uint m = v % 10;
			printf("i=%d v=%u m=%u\n", i, v, m);
			printf("b="); big_printf(&b, " ");
			big_mulDigit(&b, m);
			printf("b*m="); big_printf(&b, "\n");
			v /= 10;
		}
	}
	big_printf(&b, "\n");
}
void big_muldigitsErdos(BIG* a) {
	BIG b;
	big_init(&b, 1);
	for (int i = a->n - 1; i >= 0; i--) {
		uint v = a->t[i];
		while (v) {
			uint m = v % 10;
			if (m > 1) {
				printf("i=%d v=", i); printf(FMT0, v); printf("m = "); printf(FMT0, m); printf("\n");
				printf("b="); big_printf(&b, " ");
				big_mulDigit(&b, m);
				printf("b*m="); big_printf(&b, "\n");
			}
			v /= 10;
		}
	}
	big_printf(&b, "\n");
}
int pErdos(BIG* a, int decal = 0, bool disp = false) {
	int p = 0;
	BIG b;
	big_init(&b, 1);
	for (;;) {
		// if only one digit go out
		if (a->n == 1 && a->t[0] < 10) break;
		p++;
		b.n = 1; b.t[0] = 1; // big_init(&b, 1);
#if 0
		for (int i = a->n - 1; i >= 0; i--) {
			uint v = a->t[i];
			while (v) {
				uint m = v % 10;
				if (m > 1) {
					// printf("i=%d v=%u m=%u\n", i, v, m);
					// printf("b="); big_printf(&b, " ");
					big_mulDigit(&b, m);
					// printf("b*m="); big_printf(&b, "\n");
				}
				v /= 10;
			}
		}
#else
		int occ[10];
		for (int i = 0; i < 10; i++) occ[i] = 0;
		for (int i = a->n - 1; i >= 0; i--) {
			uint v = a->t[i];
			while (v) {
				occ[v % 10]++;
				v /= 10;
			}
		}
#if 0
		for (int i = 2; i < 10; i++) {
			for (int j = 0; j < occ[i]; j++) {
				big_mulDigit(&b, i);
			}
		}
#else
		// replace 4 by 2
		if (occ[4] > 0) {
			occ[2] += occ[4] * 2; occ[4] = 0;
		}
		// replace 25 by 0
		if (occ[2] > 1 && occ[5] > 1) {
			int m = occ[2]; if (occ[5] < m) m = occ[5];
			m--;
			occ[2] -= m; occ[5] -= m;
		}
		// replace 23 by 6
		if (occ[2] > 0 && occ[3] > 0) {
			int m = occ[2]; if (occ[3] < m) m = occ[3];
			occ[2] -= m;
			occ[3] -= m;
			occ[6] += m;
		}
		// replace 8 by 2
		/*
		if (occ[8] > 0) {
			occ[2] += occ[8] * 3; occ[8] = 0;
		}
		*/
		// replace 222 by 8
		if (occ[2] > 3) {
			int m = occ[2] / 3;
			occ[2] -= m * 3;
			occ[8] += m;
		}
		// replace 33 by 9
		if (occ[3] >= 2) {
			int m = occ[3] / 2;
			occ[3] -= m * 2;
			occ[9] += m;
		}

		// for (int i = 2; i < 10; i++) printf("occ[%d]=%d ", i, occ[i]); printf("\n");

		for (int j = 0; j < occ[2]; j++) big_mulDigitBy2(&b);
		if (occ[3]) big_mulDigit(&b, 3);    // occ[3] can only be 0 or 1
		// occ[4] = 0
		for (int i = 5; i < 10; i++) {
			for (int j = occ[i]; j > 0; j--) big_mulDigit(&b, i);
		}
#endif
#endif
		if (disp) { printf("p=%d :: ", p + decal); big_printf(&b, "\n"); }
		big_swap(a, &b); // big_copy(&b, a);
	}
	return p + decal;
}
int main(int argc, char* argv[]) {
	BIG a;
	uint m = 9;
	int n = 36;
	if (argc == 1) {
		printf("usage:\n");
		printf(" %s <nb> r [-nostat]: calcule length(Syr(n)) tous les n au hasard composes de 8*nb chiffres\n", argv[0]);
		printf(" %s <nb> R : calcule length(Syr(n)) tous les n au hasard composes de nb chiffres\n", argv[0]);
		printf(" %s s x<a><pmax> : calcule length(Syr(n)) tous les n = a ^ p avec p = [1..pmax]\n", argv[0]);
		printf(" %s <n> x y : calcule Syr(n) et length(Syr(n))\n", argv[0]);
		printf(" %s <n0> h : calcule les max de Syr(n) pour n=n0...infini\n", argv[0]);
		printf(" %s <n0> l : calcule les max de length(Syr(n)) pour n=n0...infini\n", argv[0]);
		printf(" %s <n0> b : calcule les max de Syr(n) et de length(Syr(n)) pour n=n0...infini\n", argv[0]);
		exit(1);
	}
	if (argc > 1) sscanf(argv[1], "%u", &m);
	if (argc > 2) sscanf(argv[2], "%d", &n);
	bool ok, disp = false; if (argc > 3) disp = true;
	clock_t t1, t2, t3;
	char ret[8];
	ret[0] = 13; ret[1] = 0;

	// test big_divDigitBy2
	// big_init(&a, m);
	big_init(&a);
	ok = big_setStr(&a, argv[1]);
	if (!ok) {
		fprintf(stderr, "Ooops illegal number"); exit(1);
	}
	/*
	big_printf(&a, " * 3 = ");
	big_mulDigit(&a, 3);
	big_printf(&a, "\n");
	*/
	big_dump(&a);
	big_printf(&a, " + 1 = ");
	big_inc(&a);
	big_printf(&a, "\n");
	big_dump(&a);

	big_printf(&a, " / 2 = ");
	big_divDigitBy2(&a);
	big_printf(&a, "\n");

	if (argc > 2) {
		// syracuse
		if (argv[2][0] >= '1' && argv[2][0] <= '9') {	// calcul toutes les puissances de 9
			uint p;
			sscanf(argv[2], "%u", &p);
			uint loopMax = 1000;
			if (argc > 3 && argv[3][0] >= '1' && argv[3][0] <= '9') sscanf(argv[3], "%u", &loopMax);
			printf("**** MODE pow %u\n", p);
			BIG r, b;
			big_init(&r, p);
			big_init(&b);
			for (uint loop = 0;loop<loopMax; loop++) {
				big_copy(&b, &r);
				uint nbIter = 0;
				while (!(b.n == 1 && b.t[0] == 1)) {
					nbIter++;
					if (b.t[0] & 1) {           // b=b*3+1
						big_mulDigit(&b, 3);
						big_inc(&b);
					}
					else big_divDigitBy2(&b);   // b/=2
				}
				printf("%u\t", loop+1);
				big_printf(&r);
				printf("\t%u\n", nbIter);
				big_mulDigit(&r, p);
			}
			printf("Finito\n");
			exit(0);
		}
		if (argv[2][0] == 'r') {	// a.exe [nbDigits] [r] : search for big length with random numbers of size nbDigits
			bool statsOn = true;
			uint n;
			sscanf(argv[1], "%u", &n);
			if (argc >= 3) {	// options
				for (int i = 3; i < argc; i++) {
					if (!strcmp(argv[i], "-nostat")) statsOn=false;
					else {
						fprintf(stderr, "%s: unknown option '%s'\n", argv[0], argv[i]);
						exit(1);
					}
				}
			}
			printf("**** MODE random (with %ux%d=%u digits)\n", n, BIGMAX_NBDIGITS, n*BIGMAX_NBDIGITS);
			char fLongName[256], fStatName[256];
			FILE* fLong, *fStat;
			sprintf(fLongName, "3n+1.long.%u.txt", n);
			sprintf(fStatName, "3n+1.long.stats.%u.txt", n);
			BIG r, b, bStart;
			big_init(&r, 1, n);
			big_init(&b);
			big_init(&bStart);
			uint nbIterMax = 0;
			// if fLongName file exists get max nbIterMax value
			fLong = fopen(fLongName, "r");
			if (fLong) {
				uint maxLineLength = 4000000;
				char* buf = (char*)malloc(maxLineLength * sizeof(char));	// just in case of big lines
				if (buf == (char*)NULL) {
					fprintf(stderr, "%s: no more memory trying to allocate %uMb\n", argv[0], maxLineLength/1000000);
					exit(2);
				}
				while (fgets(buf, maxLineLength, fLong)) {
					uint i = 0, v;
					while (buf[i] != '\0' && buf[i] != '\t' && buf[i] != ' ') i++;
					if (buf[i] == '\0') continue;
					i++;	// skip separator
					if (buf[i] >= '0' && buf[i] <= '9') sscanf(&buf[i], "%u", &v);
					if (v > nbIterMax) nbIterMax = v;
				}
				fclose(fLong);
				free(buf);
			}
			printf("Starting with nbIterMax=%u\n", nbIterMax);

			uint rnd;
			srand((uint)time(NULL));
			// fills r with random numbers
			// for (uint i = 0; i < n; i++) r.t[i] = ((rand() << 12) | rand()) % BIGMAX;
			for (uint i = 0; i < n; i++) { rand_s(&rnd); r.t[i] = rnd % BIGMAX; }
			r.n = n;
			// printf(" N="); big_printf(&r, "\n");
			for (uint loop = 0;; loop++) {
				big_copy(&b, &r);
				if ((b.t[0] & 1)==0) b.t[0] |= 1;	// force starting with odd number
				if (b.t[n - 1] < BIGMAX_SUR_10) b.t[n - 1] += (rand()%10)*BIGMAX_SUR_10;	// force not leading 0 (to be sure to have 8*n digits)
				big_copy(&bStart, &b);

				// printf(" loop=%u N=", loop); big_printf(&r, "\n");
				if (loop == 100000) {
					loop = 0; printf(" N="); big_printf(&bStart, ret);
				}
				uint nbIter = 0;
				while (!(b.n == 1 && b.t[0] == 1)) {
					nbIter++;
					if (b.t[0] & 1) {           // b=b*3+1
						big_mulDigit(&b, 3);
						big_inc(&b);
					}
					else big_divDigitBy2(&b);   // b/=2
				}
				if (statsOn) {
					fStat = fopen(fStatName, "a");
					fprintf(fStat, "%u\n", nbIter);
					fclose(fStat);
				}
				if (nbIter > nbIterMax) {
					nbIterMax = nbIter;
					printf(" N="); big_printf(&bStart);
					printf(" nbIter=%u\n", nbIter);
					fLong = fopen(fLongName, "a");
					big_fprintf(fLong, &bStart);
					fprintf(fLong, "\t%u\n", nbIter);
					fclose(fLong);
				}
				// big_shiftUp(&r); r.t[0] = ((rand() << 12) | rand()) % BIGMAX;
				rand_s(&rnd); big_shiftUp(&r); r.t[0] = rnd % BIGMAX;
			}
			printf("Finito\n");
			exit(0);
		}

		if (argv[2][0] == 'R') {	// a.exe [nbDigits] [r] : search for big length with random numbers of size nbDigits
			uint n;
			sscanf(argv[1], "%u", &n);
			if (n == 19) {
				uint n1 = n / BIGMAX_NBDIGITS, n2 = n % BIGMAX_NBDIGITS, n3= 1 + (n - 1) / BIGMAX_NBDIGITS;	// n3=0:1 8:1 9:2 16:2 17:3 -  n=19 => n1=2 n2=3
				uint ar;
				uint ar0 = 1; for (uint i = 1; i < n2; i++) ar0 *= 10;		// n=19 => ar0=100
				uint am = 9 * ar0;

				printf("**** MODE random (with %u-digits n)\n", n);
				char fLongName[256], fStatName[256];
				FILE* fLong, * fStat;
				sprintf(fLongName, "3n+1.long.digits.%u.txt", n);
				sprintf(fStatName, "3n+1.stat.digits.%u.txt", n);
				BIG b, bStart;
				big_init(&b);
				b.n = n3;
				printf("**** n1=%u n2=%u b.n=%u\n", n1, n2, b.n);
				big_init(&bStart);
				uint nbIterMax = 0;
				// srand((uint)time(NULL));	// not used by rand_s
				for (uint loop = 0;; loop++) {
					// big_set(&b, v);
					// 1/ full numbers
					for (uint i = 0; i < n1; i++) {
						rand_s(&ar); b.t[i] = ar % BIGMAX;
					}
					// 2/ reste
					if (n2) {
						rand_s(&ar); 
						uint v = ar0 + ar % am;
						b.t[n1] = v;
					}
					b.n = n3;
					// big_dump(&b);
					// exit(1);
					if ((b.t[0] & 1) == 0) b.t[0] |= 1;	// force starting with odd number
					big_copy(&bStart, &b);

					// printf(" loop=%u N=", loop); big_printf(&r, "\n");
					if (loop == 2000) {
						loop = 0; printf(" N="); big_printf(&bStart, ret);
					}
					uint nbIter = 0;
					while (!(b.n == 1 && b.t[0] == 1)) {
						nbIter++;
						if (b.t[0] & 1) {           // b=b*3+1
							big_mulDigit(&b, 3);
							big_inc(&b);
						}
						else big_divDigitBy2(&b);   // b/=2
					}
					fStat = fopen(fStatName, "a");
					fprintf(fStat, "%u\n", nbIter);
					fclose(fStat);
					if (nbIter > nbIterMax) {
						nbIterMax = nbIter;
						printf(" N="); big_printf(&bStart);
						printf(" nbIter=%u\n", nbIter);
						fLong = fopen(fLongName, "a");
						big_fprintf(fLong, &bStart);
						fprintf(fLong, "\t%u\n", nbIter);
						fclose(fLong);
					}
				}
				printf("Finito\n");
				exit(0);
			}
			if (n > 9) {
				fprintf(stderr, "Err: n=%u too big. must be <= 9\n", n);
				exit(1);
			}
			uint ar;
			uint ar0 = 1; for (uint i = 1; i < n; i++) ar0 *= 10;
			uint am = 9 * ar0;
			/*
			// find a random number with n digits (fot n=5 between 10000 and 99999)
			uint armin = 0xffffffff, armax = 0;
			printf("ar0=%u am=%u\n", ar0, am);
			for (int i = 1; i < 10000000; i++) {
				rand_s(&ar);
				uint v = ar0 + ar%am;
				if (v > armax) { armax = v; printf("min=%u max=%u\n", armin, armax); }
				else if (v < armin) { armin = v; printf("min=%u max=%u\n", armin, armax); }
			}
			exit(0);
			*/
			printf("**** MODE random (with %u-digits n)\n", n);
			char fLongName[256], fStatName[256];
			FILE* fLong, * fStat;
			sprintf(fLongName, "3n+1.long.digits.%u.txt", n);
			sprintf(fStatName, "3n+1.stat.digits.%u.txt", n);
			BIG b, bStart;
			big_init(&b);
			big_init(&bStart);
			uint nbIterMax = 0;
			// srand((uint)time(NULL));	// not used by rand_s
			for (uint loop = 0;; loop++) {
				rand_s(&ar);
				uint v = ar0 + ar % am;
				big_set(&b, v);
				if ((b.t[0] & 1) == 0) b.t[0] |= 1;	// force starting with odd number
				big_copy(&bStart, &b);

				// printf(" loop=%u N=", loop); big_printf(&r, "\n");
				if (loop == 100000) {
					loop = 0; printf(" N="); big_printf(&bStart, ret);
				}
				uint nbIter = 0;
				while (!(b.n == 1 && b.t[0] == 1)) {
					nbIter++;
					if (b.t[0] & 1) {           // b=b*3+1
						big_mulDigit(&b, 3);
						big_inc(&b);
					}
					else big_divDigitBy2(&b);   // b/=2
				}
				fStat = fopen(fStatName, "a");
				fprintf(fStat, "%u\n", nbIter);
				fclose(fStat);
				if (nbIter > nbIterMax) {
					nbIterMax = nbIter;
					printf(" N="); big_printf(&bStart);
					printf(" nbIter=%u\n", nbIter);
					fLong = fopen(fLongName, "a");
					big_fprintf(fLong, &bStart);
					fprintf(fLong, "\t%u\n", nbIter);
					fclose(fLong);
				}
			}
			printf("Finito\n");
			exit(0);
		}

		ok = big_setStr(&a, argv[1]);
		if (!ok) {
			fprintf(stderr, "Ooops illegal number"); exit(1);
		}

		big_printf(&a, "\n");

		BIG b;
		big_init(&b);
		BIG aMax, aMaxPrev;
		big_init(&aMax);
		big_init(&aMaxPrev);
		uint nbIterMax = 0;

		if (argc > 3) {
			big_copy(&b, &a);
			uint nbIter = 0, nbMul = 0;
			while (!(b.n == 1 && b.t[0] == 1)) {
				nbIter++;
				if (b.t[0] & 1) {           // b=b*3+1
					nbMul++;
					big_mulDigit(&b, 3);
					big_inc(&b);
					if (big_gt(&b, &aMax)) big_copy(&aMax, &b);
				}
				else big_divDigitBy2(&b);   // b/=2
				big_printf(&b, "\n");
			}
			printf("Finito. nbIter=%d nbDiv=%u nbMul=%u nbDiv/nbMul=%.4lf", nbIter, nbIter - nbMul, nbMul, (double)((double)(nbIter - nbMul) / nbMul));
			printf(" max="); big_printf(&aMax, "\n");
			exit(0);
		}
		char option = argv[2][0];
		if (option != 'h' && option != 'l' && option != 'b') {
			fprintf(stderr, "option=[height|length|both]\n");
			exit(1);
		}
		FILE* fHigh, * fLong;
		if (option == 'l' || option == 'b') {
			// load previous results
			// load nbIterMax
			fLong = fopen("3n+1.long.txt", "r");
			if (fLong) {
				char buf[2048], tmp[2048];
				strcpy(tmp, "nope");
				while (fgets(buf, 2048, fLong)) {	// load lines like "127456254\t950\n"
					for (uint i = 0; i < strlen(buf); i++) {
						if (buf[i] == '\t') {
							strcpy(tmp, buf + i + 1);
						}
					}
				}
				if (strcmp(tmp, "nope")) {
					sscanf(tmp, "%u", &nbIterMax);
					// printf("fLong::buf=%s nbIterMax=%u", buf, nbIterMax);
				}
				fclose(fLong);
			}
			printf("*** nbIterMax=%u\n", nbIterMax);
		}
		if (option == 'h' || option == 'b') {
			// load previous results
			// load nbIterMax
			fLong = fopen("3n+1.long.txt", "r");
			if (fLong) {
				char buf[2048], tmp[2048];
				strcpy(tmp, "nope");
				while (fgets(buf, 2048, fLong)) {	// load lines like "127456254\t950\n"
					for (uint i = 0; i < strlen(buf); i++) {
						if (buf[i] == '\t') {
							strcpy(tmp, buf + i + 1);
						}
					}
				}
				if (strcmp(tmp, "nope")) {
					sscanf(tmp, "%u", &nbIterMax);
					// printf("fLong::buf=%s nbIterMax=%u", buf, nbIterMax);
				}
				fclose(fLong);
			}
			fHigh = fopen("3n+1.high.txt", "r");
			if (fHigh) {
				char buf[2048], tmp[2048];
				strcpy(tmp, "nope");
				while (fgets(buf, 2048, fHigh)) {	// load lines like "59436135663\t205736389371841852168\n"
					for (uint i = 0; i < strlen(buf); i++) {
						if (buf[i] == '\t') {
							strcpy(tmp, buf + i + 1);
						}
					}
				}
				if (strcmp(tmp, "nope")) {
					for (uint j = 0; j < strlen(tmp); j++) {	// cleanup string
						if (tmp[j] == '\r' || tmp[j] == '\n') {
							tmp[j] = '\0';
							break;
						}
					}
					big_setStr(&aMaxPrev, tmp);
					// printf("fHigh::tmp='%s' send to aMaxPrev", tmp);
				}
				fclose(fHigh);
			}
			printf("*** aMax="); big_printf(&aMaxPrev, "\n");
		}

		if (option == 'b') {
			for (uint loop = 0;; loop++) {
				big_copy(&b, &a);
				if (loop == 100000) {
					loop = 0; printf(" N="); big_printf(&b, ret);
				}
				uint nbIter = 0;
				while (!(b.n == 1 && b.t[0] == 1)) {
					nbIter++;
					if (b.t[0] & 1) {           // b=b*3+1
						big_mulDigit(&b, 3);
						big_inc(&b);
						if (big_gt(&b, &aMax)) big_copy(&aMax, &b);
					}
					else big_divDigitBy2(&b);   // b/=2
				}
				if (nbIter > nbIterMax) {
					nbIterMax = nbIter;
					big_printf(&a);
					printf(" nbIter=%u\n", nbIter);
					fLong = fopen("3n+1.long.txt", "a");
					big_fprintf(fLong, &a);
					fprintf(fLong, "\t%u\n", nbIter);
					fclose(fLong);
				}
				if (big_gt(&aMax, &aMaxPrev)) {
					big_copy(&aMaxPrev, &aMax);
					big_printf(&a, " aMax=");
					big_printf(&aMax, "\n");
					fHigh = fopen("3n+1.high.txt", "a");
					big_fprintf(fHigh, &a, "\t");
					big_fprintf(fHigh, &aMax, "\n");
					fclose(fHigh);
				}
				big_inc(&a);
			}
			printf("Finito\n");
		}
		else if (option == 'h') {
			for (uint loop = 0;; loop++) {
				big_copy(&b, &a);
				if (loop == 100000) {
					loop = 0; printf(" N="); big_printf(&b, ret);
				}
				uint nbIter = 0;
				while (!(b.n == 1 && b.t[0] == 1)) {
					nbIter++;
					if (b.t[0] & 1) {           // b=b*3+1
						big_mulDigit(&b, 3);
						big_inc(&b);
						if (big_gt(&b, &aMax)) big_copy(&aMax, &b);
					}
					else big_divDigitBy2(&b);   // b/=2
				}
				if (big_gt(&aMax, &aMaxPrev)) {
					big_copy(&aMaxPrev, &aMax);
					big_printf(&a, " aMax=");
					big_printf(&aMax, "\n");
					fHigh = fopen("3n+1.high.txt", "a");
					big_fprintf(fHigh, &a, "\t");
					big_fprintf(fHigh, &aMax, "\n");
					fclose(fHigh);
				}
				big_inc(&a);
				big_inc(&a);			// only test even numbers !
			}
			printf("Finito\n");
		}
		else if (option == 'l') {
			for (uint loop = 0;; loop++) {
				big_copy(&b, &a);
				if (loop == 100000) {
					loop = 0; printf(" N="); big_printf(&b, ret);
				}
				uint nbIter = 0;
				while (!(b.n == 1 && b.t[0] == 1)) {
					nbIter++;
					if (b.t[0] & 1) {           // b=b*3+1
						big_mulDigit(&b, 3);
						big_inc(&b);
					}
					else big_divDigitBy2(&b);   // b/=2
				}
				if (nbIter > nbIterMax) {
					nbIterMax = nbIter;
					big_printf(&a);
					printf(" nbIter=%u\n", nbIter);
					fLong = fopen("3n+1.long.txt", "a");
					big_fprintf(fLong, &a);
					fprintf(fLong, "\t%u\n", nbIter);
					fclose(fLong);
				}
				big_inc(&a);		// test all numbers, odd or even
			}
			printf("Finito\n");
		}

	}

	exit(0);

	// test big_pow
	big_init(&a, m);
	big_printf(&a, " ^ ");
	BIG b;
	big_init(&b, n);
	big_printf(&b, " = ");
	t1 = clock();
	BIG c;
	big_init(&c);
	big_pow(&c, &a, n);
	t2 = clock();
	// printf("%d^%d = ", m, n); big_printf(&a, "\n");
	big_printf(&c, "\n");
	printf(" (%.3lf s.)\n", (double)(((double)t2 - (double)t1) / (double)CLOCKS_PER_SEC));

	exit(0);

	/*
	// test big_mul
	int loop = 4;
	if (argc > 3) sscanf(argv[3], "%d", &loop);

	big_init(&a, m);
	big_printf(&a, "\n");

	BIG b, c;
	big_init(&b, n);
	big_init(&c, m);
	printf("a->n=%d a->t[0]=%d\n", a.n, a.t[0]);
	printf("b->n=%d b->t[0]=%d\n", b.n, b.t[0]);

	for (int i = 0; i < loop; i++) {
		big_mul(&c, &b, &a);
		// big_printf(&a, " * ");
		// big_printf(&b, " = ");
		// big_printf(&c, "\n");
		big_copy(&b, &c);
	}
	big_printf(&a, " * ");
	big_printf(&b, " ");
	printf(" ^ %d = ", loop);
	big_printf(&c, "\n");
	exit(0);
*/

	printf("Computing %d^%d...", m, n);
	t1 = clock();
	for (int i = 1; i < n; i++) {
		// big_printf(&a);
		// printf(" * %d = ", m);
		big_mulDigit(&a, m);
		// big_printf(&a, "\n");
	}
	t2 = clock();
	// printf("%d^%d = ", m, n); big_printf(&a, "\n");
	printf("done");
	printf(" (%.3lf s.)\n", (double)(((double)t2 - (double)t1) / (double)CLOCKS_PER_SEC));

	t1 = clock();
	int p = pErdos(&a, 1, disp);
	t2 = clock();
	printf("p=%d (%.3lf s.)\n", p, (double)(((double)t2 - (double)t1) / (double)CLOCKS_PER_SEC));
	exit(1);

	printf("// sizeof(BIG)=%zd\n", sizeof(a));
	printf("pE(9^%d)=", n);
	printf("[first]");
	t1 = clock();
	for (int i = 0; i < n - 1; i++) {
		big_mulDigit(&a, m);
	}
	printf("[next]");
	// printf("9^%d=", n);
	// big_dump(&a, "\n");
	// big_muldigitsErdos(&a);
	// printf("p=0 :: "); big_printf(&a, "\n");
	t2 = clock();
	int pE = pErdos(&a, disp) + 1;
	t3 = clock();
	printf("%d\n", pE);
	printf(" (%.3lf s.)", (double)(((double)t2 - (double)t1) / (double)CLOCKS_PER_SEC));
	printf(" (%.3lf s.)", (double)(((double)t3 - (double)t1) / (double)CLOCKS_PER_SEC));
}
