#include <sstream>
#include <vector>
#include <time.h>
#include "common.hpp"
#include "vectorized.hpp"
#include "tpch_kit.hpp"

#include "kernel_naive.hpp"
#include "kernel_naive_compact.hpp"
#include "kernel_hyper.hpp"
#include "kernel_hyper_compact.hpp"
#include "kernel_x100.hpp"
#include "kernel_x100_old.hpp"
#include "kernel_avx512.hpp"
// #include "kernel_weld.hpp"

#include <tuple>

static const size_t REP_COUNT = 1;

static int runIdCounter = 0;

template<typename F, typename... Args>
void run(const lineitem& li, const std::string& name, Args&&... args)
{
	F fun(li, args...);
	size_t total_time = 0;
	double total_millis = 0.0;

	const size_t n = li.l_extendedprice.cardinality;

	for (size_t rep=0; rep<REP_COUNT; rep++) {
		clear_tables();

		timespec ts_start, ts_end;
		clock_gettime(CLOCK_MONOTONIC, &ts_start);
		const auto start = get_cycles();

		fun();

		const auto time = get_cycles() - start;
		clock_gettime(CLOCK_MONOTONIC, &ts_end);
		double million = 1000000.0;
		double millisec = (ts_end.tv_sec - ts_start.tv_sec) * 1000.0 + ((ts_end.tv_nsec - ts_start.tv_nsec) / million);

		if (true || rep != 0) { /* Throw cold run away */
			total_time += time;
			total_millis += millisec;
		}
	}

	const double hot_reps = REP_COUNT;
	const double total_tuples = hot_reps * n;

	printf("%f\n",
		(double)total_millis / 1000.0);

	runIdCounter++;
	fun.Profile(total_tuples);
#ifdef PRINT_RESULTS_NO

	auto print_dec = [] (auto s, auto x) { printf("%s%ld.%ld", s, Decimal64::GetInt(x), Decimal64::GetFrac(x)); };
	// printf("# returnflag|linestatus|sum_qty|sum_base_price|sum_disc_price|sum_charge|count_order\n");

	auto sum_64 = [] (auto& arr, size_t i, size_t n) {
		int64_t start = 0;
		for (size_t k=i; k<i+n; k++) {
			start += arr[k];
		}
		return start;
	};

	auto sum_128 = [] (auto& hi, auto& lo, size_t i, size_t n) {
		__int128 start = 0;
		for (size_t k=i; k<i+n; k++) {
			auto h = (__int128)hi[k] << 64;
			auto l = (__int128)lo[k];
			start += h | l;
		}
		return start;
	};

	for (size_t group=0; group<MAX_GROUPS; group++) {
		if (aggrs0[group].count > 0) {
			char rf = group >> 8;
			char ls = group & std::numeric_limits<unsigned char>::max();

			size_t i = group;

			printf("# %c|%c", rf, ls);
			print_dec("|", aggrs0[i].sum_quantity);
			print_dec("|", aggrs0[i].sum_base_price);
			print_dec("|", aggrs0[i].sum_disc_price);
			print_dec("|", aggrs0[i].sum_charge);
			printf("|%ld\n", aggrs0[i].count);
		}
	}
	for (size_t group=0; group<MAX_GROUPS; group++) {
		if (aggrs1[group].count > 0) {
			char rf = group >> 8;
			char ls = group & std::numeric_limits<unsigned char>::max();

			size_t i = group;

			printf("# %c|%c", rf, ls);
			print_dec("|", aggrs1[i].sum_quantity);
			print_dec("|", aggrs1[i].sum_base_price);
			print_dec("|", aggrs1[i].sum_disc_price);
			print_dec("|", aggrs1[i].sum_charge);
			printf("|%ld\n", aggrs1[i].count);
		}
	}
	size_t i=0;
	for (size_t group=0; group<MAX_GROUPS; group++) {
		char rf = group >> 8;
		char ls = group & std::numeric_limits<unsigned char>::max();

		int64_t count = sum_64(aggr_avx0_count, i, 8);

		if (count > 0) {
			char rf = group >> 8;
			char ls = group & std::numeric_limits<unsigned char>::max();

			printf("# %c|%c", rf, ls);
			print_dec("|", sum_64(aggr_avx0_sum_quantity, i, 8));
			print_dec("|", sum_64(aggr_avx0_sum_base_price, i, 8));
			print_dec("|", sum_128(aggr_avx0_sum_disc_price_hi, aggr_avx0_sum_disc_price_lo, i, 8));
			print_dec("|", sum_128(aggr_avx0_sum_charge_hi, aggr_avx0_sum_charge_lo, i, 8));
			printf("|%ld\n", count);
		}

		i+=8;
	}
#endif
}

#include "common.hpp"

struct KernelNaiveCompactSF1Opt : BaseKernel {
	kernel_compact_declare

	KernelNaiveCompactSF1Opt(const lineitem& li) : BaseKernel(li) {
		kernel_compact_init();
	}


	typedef struct { uint64_t lo; uint64_t hi; } emu128;

	template<typename T>
	void sum_128(__int128_t* dest, T val) {
		emu128* d = (emu128*)dest;
		emu128 tmp = *d;

		tmp.lo += val;

		if (UNLIKELY(tmp.lo < val)) {
			tmp.hi++;
			*d = tmp;
		} else {
			int64_t* d64 = (int64_t*)dest;
			*d64 = tmp.lo;
		}
	}

	__attribute__((noinline)) void operator()() noexcept {
		//kernel_small_dt_prologue();
		kernel_prologue();

		const int8_t one = Decimal64::ToValue(1, 0);
		const int16_t date = cmp.dte_val;

		for (size_t i=0; i<cardinality; i++) {
			if (l_shipdate[i] <= date) {
				const auto disc = l_discount[i];
				const auto price = l_extendedprice[i];
				const int8_t disc_1 = one - disc;
				const int8_t tax_1 = tax[i] + one;
				const int32_t disc_price = disc_1 * price;
				const int64_t charge = (int64_t)disc_price * tax_1; // seems weird but apparently this triggers a multiplication with 64-bit result
				const int16_t idx = (int16_t)(returnflag[i] << 8) | linestatus[i];
				aggrs1[idx].sum_quantity += (int64_t)l_quantity[i];
				aggrs1[idx].sum_base_price += (int64_t)price;
				aggrs1[idx].sum_disc_price += disc_price;
				aggrs1[idx].sum_charge += charge;
				aggrs1[idx].sum_disc += disc;
				aggrs1[idx].count++;
			}
		}
	}
};


template<bool use_flag>
struct KernelHyPerCompactSF1Opt : BaseKernel {
	KernelHyPerCompactSF1Opt(const lineitem& li) : BaseKernel(li) {
		kernel_compact_init();
	}

	kernel_compact_declare

	__attribute__((noinline)) void operator()() noexcept {
		kernel_prologue();

		const int8_t one = Decimal64::ToValue(1, 0);
		const int16_t date = cmp.dte_val;

		int8_t disc_1;
		int8_t tax_1;
		int32_t disc_price;
		int64_t charge;

		if (!use_flag) {
			for (size_t i=0; i<cardinality; i++) {
				if (l_shipdate[i] <= date) {
					const auto disc = l_discount[i];
					const auto price = l_extendedprice[i];

					if (UNLIKELY(__builtin_sub_overflow(one, disc, &disc_1))) {
						handle_overflow();
					}

					if (UNLIKELY(__builtin_add_overflow(one, l_tax[i], &tax_1))) {
						handle_overflow();
					}

					if (UNLIKELY(__builtin_mul_overflow(disc_1, price, &disc_price))) {
						handle_overflow();
					}
					if (UNLIKELY(__builtin_mul_overflow(disc_price, tax_1, &charge))) {
						handle_overflow();
					}
					const int16_t idx = (int16_t)(returnflag[i] << 8) | linestatus[i];
					if (UNLIKELY(__builtin_add_overflow(aggrs1[idx].sum_quantity, l_quantity[i], &aggrs1[idx].sum_quantity))) {
						handle_overflow();
					}
					if (UNLIKELY(__builtin_add_overflow(aggrs1[idx].sum_base_price, price, &aggrs1[idx].sum_base_price))) {
						handle_overflow();
					}

					if (UNLIKELY(__builtin_add_overflow(aggrs1[idx].sum_disc_price, disc_price, &aggrs1[idx].sum_disc_price))) {
						handle_overflow();
					}

					if (UNLIKELY(__builtin_add_overflow(aggrs1[idx].sum_charge, charge, &aggrs1[idx].sum_charge))) {
						handle_overflow();
					}

					if (UNLIKELY(__builtin_add_overflow(aggrs1[idx].sum_disc, disc, &aggrs1[idx].sum_disc))) {
						handle_overflow();
					}
					if (UNLIKELY(__builtin_add_overflow(aggrs1[idx].count, 1, &aggrs1[idx].count))) {
						handle_overflow();
					}
				}
			}
		} else {
			bool flag = false;
			for (size_t i=0; i<cardinality; i++) {
				if (l_shipdate[i] <= date) {
					const auto disc = l_discount[i];
					const auto price = l_extendedprice[i];

					flag |= __builtin_sub_overflow(one, disc, &disc_1);
					flag |= __builtin_add_overflow(one, l_tax[i], &tax_1);

					flag |= __builtin_mul_overflow(disc_1, price, &disc_price);
					flag |= __builtin_mul_overflow(disc_price, tax_1, &charge);

					const int16_t idx = (int16_t)(returnflag[i] << 8) | linestatus[i];
					flag |= __builtin_add_overflow(aggrs1[idx].sum_quantity, l_quantity[i], &aggrs0[idx].sum_quantity);
					flag |= __builtin_add_overflow(aggrs1[idx].sum_base_price, price, &aggrs0[idx].sum_base_price);

					flag |= __builtin_add_overflow(aggrs1[idx].sum_disc_price, disc_price, &aggrs0[idx].sum_disc_price);
					flag |= __builtin_add_overflow(aggrs1[idx].sum_charge, charge, &aggrs0[idx].sum_charge);

					flag |= __builtin_add_overflow(aggrs1[idx].sum_disc, disc, &aggrs0[idx].sum_disc);
					flag |= __builtin_add_overflow(aggrs1[idx].count, 1, &aggrs0[idx].count);
				}
			}

			if (flag) {
				handle_overflow();
			}
		}

		kernel_epilogue();
	}
};


int main(int argc, char* argv[]) {
	std::ios::sync_with_stdio(false);

	/* load data */
	lineitem li(700000000ull);
	li.FromFile("lineitem.tbl");

	/* start processing */
#if 0
	printf("ID \t %-40s \t timetuple \t millisec \t aggrtuple \t pshuffletuple \t remainingtuple\n",
		"Configuration");
#endif
//	run<KernelWeld>(li, "$\\text{Weld}$");

#if 0
	run<KernelOldX100<kMultiplePrims, true, kSinglePrims, false>>(li, "$\\text{X100 Full NSM Standard}$");
	run<KernelOldX100<kMultiplePrims, false, kSinglePrims, false>>(li, "$\\text{X100 Full DSM Standard}$");
	run<KernelOldX100<k1Step, true, kSinglePrims, false>>(li, "$\\text{X100 Full NSM Standard Fused}$");
	run<KernelOldX100<kMagic, true, kSinglePrims, true>>(li, "$\\text{X100 Full NSM In-Reg}$");

	run<KernelX100<kMultiplePrims, true>>(li, "$\\text{X100 Compact NSM Standard}$");
	run<KernelX100<kMultiplePrims, false>>(li, "$\\text{X100 Compact DSM Standard}$");
	run<KernelX100<k1Step, true>>(li, "$\\text{X100 Compact NSM Standard Fused}$");
	run<KernelX100<kMagic, true>>(li, "$\\text{X100 Compact NSM In-Reg}$");

#ifdef __AVX512F__
	run<KernelX100<kMagic, true, kPopulationCount>>(li, "$\\text{X100 Compact NSM In-Reg AVX-512}$");
	run<KernelX100<kMagic, true, kCompare>>(li, "$\\text{X100 Compact NSM In-Reg AVX-512 Cmp}$");
#endif

#endif
	switch (argc) {
	case 1:
		for (size_t i=0; i<10; i++) run<KernelNaiveCompactSF1Opt>(li, "$\\text{HyPer Compact OverflowPrevent SF1 opt}$");
		break;

	case 2:
		for (size_t i=0; i<10; i++) run<KernelHyPerCompactSF1Opt<false>>(li, "$\\text{HyPer Compact OverflowBranch SF1 opt}$");
		break;

	case 3:
		for (size_t i=0; i<10; i++) run<KernelNaiveCompact>(li, "$\\text{HyPer Compact Opt for OverflowPrevent Sf100}$");
		break;

	case 4:
		for (size_t i=0; i<10; i++) run<KernelHyPerCompact<false>>(li, "$\\text{HyPer Compact Opt for OverflowBranch Sf100}$");
		break;
	}

#if 0
	for (size_t i=0; i<10; i++)
	run<KernelHyPer<true>>(li, "$\\text{HyPer Full}$");
	for (size_t i=0; i<10; i++)
	run<KernelHyPer<false>>(li, "$\\text{HyPer Full OverflowBranch}$");
	for (size_t i=0; i<10; i++)
	run<KernelNaive>(li, "$\\text{HyPer Full NoOverflow}$");


	for (size_t i=0; i<10; i++)
	run<KernelHyPerCompact<true>>(li, "$\\text{HyPer Compact}$");
	for (size_t i=0; i<10; i++)
	run<KernelHyPerCompact<false>>(li, "$\\text{HyPer Compact OverflowBranch}$");
#endif

#ifdef __AVX512F__
	run<AVX512<false, false, true>>(li, "$\\text{Handwritten AVX-512}$");
	run<AVX512<false, false, false>>(li, "$\\text{Handwritten AVX-512 Only64BitAggr}$");
#endif
	return 0;
}
