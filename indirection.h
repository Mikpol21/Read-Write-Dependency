#include <vector>
#include <span>
#include <stdint.h>

uint64_t simple_iterate(const std::vector<uint32_t> &A, const std::vector<uint32_t> &B)
{
    uint64_t total = 0;
    for (uint32_t a : A)
        total += B[a];
    return total;
}

template <const uint32_t block_size>
uint64_t iterate_with_prefetch(const std::vector<uint32_t> &A, const std::vector<uint32_t> &B, std::span<uint32_t, block_size> prefetch_buffer)
{
    const uint32_t A_size = A.size();
    uint64_t total = 0;
    uint32_t block_start = 0;
    for (; block_start + block_size < A_size; block_start += block_size)
    {
        // prefetching
        for (uint32_t i = 0; i < block_size; ++i)
            prefetch_buffer[i] = B[A[block_start + i]];

        // sum
        for (uint32_t i = 0; i < block_size; ++i)
            total += prefetch_buffer[i];
    }

    // handle the leftover block
    const uint32_t leftover_block_size = A_size - block_start;
    for (uint32_t i = 0; i < leftover_block_size; ++i)
        prefetch_buffer[i] = B[A[block_start + i]];

    for (uint32_t i = 0; i < leftover_block_size; ++i)
        total += prefetch_buffer[i];

    return total;
}

struct SplitAndScore
{
    uint64_t splitValue{};
    double score{};
};

using Derivative = std::pair<double, double>;

SplitAndScore enumerate_splits(const std::span<uint32_t> sorted_indices, const std::span<Derivative> g)
{
    constexpr double G = 1024, H = 1 << 15;
    constexpr double lambda = 0.5;

    double G_L = 0, G_R = G;
    double H_L = 0, H_R = H;

    SplitAndScore best_so_far{};

    for (auto idx : sorted_indices)
    {
        G_L += g[idx].first, G_R -= g[idx].first;
        H_L += g[idx].second, H_R -= g[idx].second;
        const double score =
            G_L * G_L / (H_L + lambda) +
            G_R * G_R / (H_R + lambda) -
            G * G / (H + lambda);

        if (score < best_so_far.score)
            best_so_far = {idx, score};
    }
    return best_so_far;
}

template <const uint32_t block_size>
SplitAndScore enumerate_splits_with_prefetch(const std::span<uint32_t> sorted_indices, const std::span<Derivative> g, std::span<Derivative, block_size> prefetch_buffer)
{
    constexpr double G = 1024, H = 1 << 15;
    constexpr double lambda = 0.5;

    double G_L = 0, G_R = G;
    double H_L = 0, H_R = H;

    SplitAndScore best_so_far{};

    for (uint32_t block_start = 0; block_start + block_size < sorted_indices.size(); block_start += block_size)
    {
        for (uint32_t idx = 0; idx < block_size; ++idx)
            prefetch_buffer[idx] = g[block_start + idx];

        for (uint32_t idx = 0; idx < block_size; ++idx)
        {
            G_L += prefetch_buffer[idx].first, G_R -= prefetch_buffer[idx].first;
            H_L += prefetch_buffer[idx].second, H_R -= prefetch_buffer[idx].second;
            const double score =
                G_L * G_L / (H_L + lambda) +
                G_R * G_R / (H_R + lambda) -
                G * G / (H + lambda);

            if (score < best_so_far.score)
                best_so_far = {sorted_indices[idx], score};
        }
    }
    return best_so_far;
}

template <const uint32_t block_size>
SplitAndScore enumerate_splits_with_prefetch2(const std::span<uint32_t> sorted_indices, const std::span<double> g, const std::span<double> h)
{
    constexpr double G = 1024, H = 1 << 15;
    constexpr double lambda = 0.5;

    double G_L = 0, G_R = G;
    double H_L = 0, H_R = H;

    SplitAndScore best_so_far{};
    std::array<double, block_size> g_cache{}, h_cache{};

    for (uint32_t block_start = 0; block_start + block_size < sorted_indices.size(); block_start += block_size)
    {
        for (uint32_t idx = 0; idx < block_size; ++idx)
        {
            g_cache[idx] = g[block_start + idx];
            h_cache[idx] = h[block_start + idx];
        }

        for (uint32_t idx = 0; idx < block_size; ++idx)
        {
            G_L += g_cache[idx], G_R -= g_cache[idx];
            H_L += h_cache[idx], H_R -= h_cache[idx];
            const double score =
                G_L * G_L / (H_L + lambda) +
                G_R * G_R / (H_R + lambda) -
                G * G / (H + lambda);

            if (score < best_so_far.score)
                best_so_far = {sorted_indices[idx], score};
        }
    }
    return best_so_far;
}
// 4 seems the best
// godbolt: https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGISQBspK4AMngMmAByPgBGmMT%2BABykAA6oCoRODB7evv5BaRmOAmER0SxxCZLJdpgOWUIETMQEOT5%2BgbaY9sUMjc0EpVGx8Um2TS1teZ0KE4PhwxWj1QCUtqhexMjsHOYAzOHI3lgA1CZ7bs3ETACe59gmGgCC%2B4fHmGcXCimG948vZgODCOXlO5zcs3wggAdAg/s9/rNiF4HCchClaIQngx0CISJh/iYAOxWZ4ncknLzhAgBSQAfQIJ2%2BmIIADUxF4CSTiQARc6kp4Uk7oDYxehMtDELlWIl8vYC3n8wnPLwZIwnHnxPAANyYjm1H3OPKZBHQIBAPzwxHBIq8YswpGFovof3lyqeBEwLAxesNFzQDFmlOpezMDJOYtQyAA1nSMgAvAl7B7PdEs7G4yUfRg%2BeK%2BuMYwgKOkAd0ICDpKSlVEwBGQCAgAaDkPN31%2BFypglDDPuTJInvQdPC%2BG2CkdTcZLZAbYY4M1xB1ep1SewJ2AjqnM7nWt1%2BodEdoUdjCZXJyrmBrdYrMS8VBrxBWhO5ZIpE8wqirTrt4oA4p9jVwGhmNIJwABL/icXCfG44KQQArEqL7km%2BH7EF%2B9onAYFToEwEEaNCCFugiSHob%2BdIhHhjo/nSABKEE/ohgoUraGGgeRlFgbREGgYx/xCmmWI4niUoRpgsxxqgdJUM0xIynKCokfwaEQJ2BDdoykYxnGcx4fyB5HtpAxnJY%2BlaSe0F9i0mCDsOeCjtCJ4QI%2B8qmceOnWEarlxngiaPiRsl8UKFJKScKkhmGjJ4Ogqi6S5UUxbBmnHj5SYWMZViWPFflMUFQXnpe9Z0jed7xCYcEWPFZXGp5wBlRYSWGS06UnJVcHye6uXkiFYVdhFLXRbFaXxRZDUnnpHmZdF2WdWcz45TNJzURRHl7Ma%2BW1oVxX3nVrU8tCNDELMVFcQAtJ561XkVt7beVu37VasyMQtFJsctljndWG3XtdpW3dFVUOXUAjoI6bF0Wdq1np9l1bb9FX/W1gMBugT3PScE6kR8ChZv%2BgVoxSS0nAAVIt7EAPSha9zVYTEOErOleP46TdEk9RdEUxAYPU6wtNMPTZ0kfjf6sycHPgdYmE83TvGC51eBUKF2P4iNYkEBJUnNA5WbTUz5JxOJCiSdJaGebJhtWTZOJ2WJO0IzyG5ZoqRHzUKir%2BbKjNSgQmwMKJBtGzJztuy8qaFgQGbCR8VCNgIzamq2Pyzh24U9smlkDkOVujuOseTvH06J9uC67suvbANlAUkVOVy3EXi57o6XABL2sPWs7Qpez7Jw5mw1yegWLLFmWBAVhd9bgk39wQObGe2dna6Oq3zkKh7xEh08HBrLQnBwbwfjcLwqCcDB70mYbmzbMZew8KQBCaJvazRiAcFcNCexwWYZiJESACcGg/wELgew9g/zgvoTgkg9731IEfDgvAFAgA0Lfe%2Baw4CwCQGgb0dB4jkEoJglI2CEjACAVwPgdBPSHUoDEaBMRwjNBuJwG%2BtDmDEBuAAeRiNoOod8D6kEwWwQQbCGC0AYRwLQpAsA3mAJcWgtAEG8KwCwQwwBxBiN4PgKU9QDTyPEe%2BOoXhPSMN4NSbo0DMQxGuKwjwWBoEEAXCwIxpADTEBiOkTAmolFGExEYFBfADDAAUKyPAmASxsJSIwRx/BBAiDEOwKQMhBCKBUOoNRpBdCkIMD40wp8bDmIQbAZgbAQBxGBAgJRxBoxONGNwAIZgH4wJSL0eRJ1ynj1WgoaMNwDDRg%2BCdNhewTgnUhEabJGULBmHEagZxC4sD5Kcl0HoWQXA4imH4UhoQFjlEqHoQomQBCrJ2ekPZDAhhbNGKQ2o9QBD9EmJ4doehLm9BufMMoIwEgXLmAcj5AxTlvIkGsc%2BWw4lbx3lA1JsCTiqESAEE6tI1zIGQJBPY0IoIQFwIQEgV8uArF4DwrQKxH5JGhLSIkRIuATL/nsaFtTwEcEgaQfekzODwMQcgtRBLaV1IZdA2BuKUFrGcRkZwkggA

// again 4 the best for prefetch2
// godbolt:
// https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGIM6SuADJ4DJgAcj4ARpjEIACsGqQADqgKhE4MHt6%2B/ilpGQIhYZEsMXGJtpj2jgJCBEzEBNk%2BfgF2mA6Z9Y0ExRHRsQlJCg1NLbntY32hA2VDiQCUtqhexMjsHOYAzKHI3lgA1Cbbbo3ETACeJ9gmGgCCO3sHmMenCsmGN3ePZrsM%2By8RxOblG%2BEEADoEN8Hj9RsQvA5DkJkrRCPcGOgRCRMD8TAB2KwPQ4kw5eUIEABskgA%2BgRDh80QQAGpiLy4wkEgAiJyJ91Jh3Qqyi9AZaGIHKs%2BJ52z53N5eIeXnSRkOXNieAAbkxHJrXicuQyCOgQCBPnhiCChV4RZhSILhfRvrLFfcCJgWKidfrTmgGKMyRTtmY6YcRahkABrGnpABeuO2tweKKZGKx4tejB8sW9MdRhAUNIA7oQEDTkhKqJgCMgEBA/QGwaaPl9TuTBMG6TcGSR3egaaF8BsFPaG/SmyAWwwrY6E9hDsBRwJG8bm59p6drbbuwhFnjOcTSWPMKoKw6baKAOJvQ1cDRmaSHAASN8OXDebhBb/iCsPJOPp7EOetqHAYZToEwr4aBCP4urCf7AVeNJBFB9qXjSABKr6Xr%2B/KkluopPshqHPphr5PrhPwCim6KYtiEphpgowxqgNJUI0BJSjKfIChO5xXDOF52mGtARtGcZzguNKiLWkrcvaZYyQgcnSpRCH8EBEDtgQnb0uGUYxtMUG8iJYmGb0xyWKZBkSR%2BPZNJg/aDngw4QhJEB7rK1niUZ1gGt5MZ4PGe4IZxVECqSGmHFpQYhvSeDoKoxleQlSVfvp4lBQmFiWVYlipSFeERccB5FcVpLANJTCySY8QWKltWGv5wC1RYGXmU0uWHA18TceF5Wkop1XKa1PVNdshrQnV7WjBZ1jdYljW4QN8rwWVApRTFHZxQtaUTYcGgmaldkzVlJl%2BfliWFStpUDQK6EoX5%2B2VUpuJ1WNaFkQAtM1VU1e9i29ctd0kkRj2WP5Q3/fVgNcvaRFYT9%2B1QyNAOqEtcHreVY6Ia8CgZje/Ug6SD2HAAVIcpMAPTRWDXVgVEEGLLlRPE5TZEU%2BhWE0xACP06wjNMMzP0IWz16c4cPMvvNDNM2pWPFXgVDRfjOInUxBAsWxjRuRm11s6SMTMQorHsUB/mcSbDlOZiLlMaNsP2qrErypjN08qFqkIRKBBrAwjHG6bHGY6tjzJvmBBpvRrxUPWy7jquk7riC2m6d2Vt9gOtvDku/oJyaSetm4BGSYuhw4xOU6CduiaHLu%2B48aSPt%2B4cWZsBc7p5kyhYlgQZYVpgVY1tCpySDcEAZ45WdDkx9pl/XIde2Hy8cMstCcPEvB%2BNwvCoJwn4Q1ZJtrBslnbDwpAEJoq/LJGCRcBC2zxGYZgABz4gAnBoH%2BUlw2zbB/eI%2BhOCSC3tfUge8OC8AUCAJIV8OBaGWHAWASA0CejoLEcglA0HJAwXEYAf8uB8DoO6YgMCIBRHAVEUIjRLicAvtQ5gxBLgAHkojaE6PAi%2BaC2CCBYQwWgdCEG8CwFELwwAzi0FoDAnepAsAsEMMAcQwi5EWk4VqJi4CTydC8O6ehvAKTVHAWiKIFxmEeCwOAggxA8AsH0aQPUxAohpEwOqBRRg0RGGvssKgBhgAKGZHgTARYWHJEYPY/gggRBiHYFIGQghFAqHUCo3QRCDBeNMIfGwJiYGwGYGwEAMQAQIAUcQSMDihjcEpGYG%2BEDki1H9JwL6pTawGgUJGS4BhIyvC%2Biw7YhwvpggNJkvKFgzBaAgY4mxWBckeSqDUTILhMQTD8EQ4IsxSjlD0KkdIDSVnbIKA0/omyhhEI6F0Oo0x9lnOqOoy5vRjmDDiGcq5nhWh6Fmk0R58xnnLGPusWJa8N5gJUZAw4qhX6Ui%2BtSBcyBkBvm2BCd8EBcCEBIGfLgixeDwMQbfEAkhX4QmpPifEXBxlf22JC6pwCOCgNINvCZkDoGwMvt4oFHAan0vAUy1lwjFjLEcekZwkggA