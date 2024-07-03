#include <algorithm>
#include <array>
#include <random>
#include "indirection.h"
#include "benchmark/benchmark.h"

std::vector<uint32_t> permutation(const uint32_t size)
{
    std::vector<uint32_t> result(size);
    for (uint32_t i = 0; i < size; ++i)
        result[i] = i;
    std::random_shuffle(result.begin(), result.end());
    return result;
}

template <typename T>
std::vector<T> random_data(const uint32_t size)
{
    std::vector<T> result(size);
    for (uint32_t i = 0; i < size; ++i)
        result[i] = std::rand();
    return result;
}

static void BM_simple_iterate(benchmark::State &state)
{
    const uint32_t data_size = state.range(0);
    std::vector<uint32_t> A = permutation(data_size);
    std::vector<uint32_t> B = random_data<uint32_t>(data_size);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(simple_iterate(A, B));
    }
}

template <const uint32_t block_size>
static void BM_iterate_with_prefetch(benchmark::State &state)
{
    const uint32_t data_size = state.range(0);
    std::vector<uint32_t> A = permutation(data_size);
    std::vector<uint32_t> B = random_data<uint32_t>(data_size);

    std::array<uint32_t, block_size> buffer{};
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(iterate_with_prefetch<block_size>(A, B, buffer));
    }
}

static void BM_enumarate(benchmark::State &state)
{
    const uint32_t data_size = state.range(0);
    std::vector<uint32_t> A = permutation(data_size);
    std::vector<Derivative> B(data_size);

    for (uint32_t i = 0; i < data_size; ++i)
        B[i] = {std::rand(), std::rand()};

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(enumerate_splits(
            std::span<uint32_t>(A.begin(), A.end()),
            std::span<Derivative>(B.begin(), B.end())));
    }
}

template <const uint32_t block_size>
static void BM_enumarate_with_prefetch(benchmark::State &state)
{
    const uint32_t data_size = state.range(0);
    std::vector<uint32_t> A = permutation(data_size);
    std::vector<Derivative> B(data_size);

    for (uint32_t i = 0; i < data_size; ++i)
        B[i] = {std::rand(), std::rand()};

    std::array<Derivative, block_size> buffer{};
    for (auto _ : state)
    {
        benchmark::DoNotOptimize(enumerate_splits_with_prefetch<block_size>(
            std::span<uint32_t>(A.begin(), A.end()),
            std::span<Derivative>(B.begin(), B.end()), buffer));
    }
}

template <const uint32_t block_size>
static void BM_enumarate_with_prefetch2(benchmark::State &state)
{
    const uint32_t data_size = state.range(0);
    std::vector<uint32_t> A = permutation(data_size);
    std::vector<double> g = random_data<double>(data_size), h = random_data<double>(data_size);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(enumerate_splits_with_prefetch2<block_size>(
            std::span<uint32_t>(A.begin(), A.end()),
            std::span<double>(g.begin(), g.end()),
            std::span<double>(h.begin(), h.end())));
    }
}

constexpr uint64_t data_size_lowerbound = 2 << 5;
constexpr uint64_t data_size_upperbound = 2 << 24;

BENCHMARK(BM_enumarate)->Range(data_size_lowerbound, data_size_upperbound);
BENCHMARK(BM_enumarate_with_prefetch<4>)->Range(data_size_lowerbound, data_size_upperbound);
BENCHMARK(BM_enumarate_with_prefetch2<4>)->Range(data_size_lowerbound, data_size_upperbound);
BENCHMARK(BM_enumarate_with_prefetch<8>)->Range(data_size_lowerbound, data_size_upperbound);
BENCHMARK(BM_enumarate_with_prefetch2<8>)->Range(data_size_lowerbound, data_size_upperbound);

BENCHMARK_MAIN();