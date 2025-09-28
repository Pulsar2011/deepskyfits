#ifndef _DSL_FITSstatistic_
#define _DSL_FITSstatistic_

#include <vector>
#include <valarray>
#include <functional>
#include <algorithm> // <-- added for std::min

#include <thread>
#include <mutex>
#include <future>

#include <Minuit2/FCNBase.h>
#include <Minuit2/MinimumBuilder.h>
#include <Minuit2/FunctionMinimum.h>
#include <Minuit2/MnUserParameterState.h>
#include <Minuit2/MnMigrad.h>
#include <Minuit2/MnSimplex.h>
#include <Minuit2/MnMinos.h>
#include <Minuit2/MnContours.h>
#include <Minuit2/MnPlot.h>
#include <Minuit2/MnPrint.h>

namespace DSL
{
    namespace stat
    {
        class Percentil: public ROOT::Minuit2::FCNBase
        {
        private:

            std::size_t lower_bound(const double& v) const;

            template<typename Src>
            static std::valarray<double> to_valarray(const std::vector<Src>& v)
            {
                const size_t n = v.size();
                std::valarray<double> out(n);
                if (n == 0) return out;

                const Src* src = v.data();
                double* dst    = &out[0];

                // fast-path when already double: use valarray pointer constructor (single memcpy-like copy)
                if (std::is_same<Src, double>::value)
                {
                    out= std::valarray<double>(reinterpret_cast<const double*>(src), n);
                    std::sort(&out[0], &out[0] + out.size());
                    return out;
                }

                unsigned int hw_threads = std::thread::hardware_concurrency();
                unsigned int nthreads = (hw_threads == 0) ? 1 : hw_threads;
                if (static_cast<size_t>(nthreads) > n) nthreads = static_cast<unsigned int>(n);

                // single thread -> simple loop (avoid async overhead)
                if (nthreads <= 1)
                {
                    for (size_t k = 0; k < n; ++k) dst[k] = static_cast<double>(src[k]);
                    std::sort(&out[0], &out[0] + out.size());
                    return out;
                }

                const size_t chunk = (n + nthreads - 1) / nthreads;
                std::vector< std::future<void> > futs;
                futs.reserve(nthreads);

                for (unsigned int ti = 0; ti < nthreads; ++ti)
                {
                    const size_t start = ti * chunk;
                    const size_t end = std::min(n, start + chunk);
                    if (start >= end) continue;

                    // capture raw pointers and bounds by value
                    futs.emplace_back(std::async(std::launch::async,
                        [src, dst, start, end]() {
                            for (size_t k = start; k < end; ++k)
                                dst[k] = static_cast<double>(src[k]);
                        }));
                }

                for (auto &f : futs) f.get();
                std::sort(&out[0], &out[0] + out.size());

                return out;
            }

            template<typename Src>
            static std::valarray<double> to_valarray(const std::valarray<Src>& v)
            {
                const size_t n = v.size();
                std::valarray<double> out(n);
                if (n == 0) return out;

                const Src* src = &v[0];
                double* dst    = &out[0];

                // fast-path when already double: use valarray pointer constructor (single memcpy-like copy)
                if (std::is_same<Src, double>::value)
                {
                    out = std::valarray<double>(reinterpret_cast<const double*>(src), n);
                    std::sort(&out[0], &out[0] + out.size());
                    return out;
                }

                unsigned int hw_threads = std::thread::hardware_concurrency();
                unsigned int nthreads = (hw_threads == 0) ? 1 : hw_threads;
                if (static_cast<size_t>(nthreads) > n) nthreads = static_cast<unsigned int>(n);

                // single thread -> simple loop (avoid async overhead)
                if (nthreads <= 1)
                {
                    for (size_t k = 0; k < n; ++k) dst[k] = static_cast<double>(src[k]);
                    std::sort(&out[0], &out[0] + out.size());
                    return out;
                }

                const size_t chunk = (n + nthreads - 1) / nthreads;
                std::vector<std::future<void>> futs;
                futs.reserve(nthreads);

                for (unsigned int ti = 0; ti < nthreads; ++ti)
                {
                    const size_t start = ti * chunk;
                    const size_t end = std::min(n, start + chunk);
                    if (start >= end) continue;

                    // capture pointers and bounds by value
                    futs.emplace_back(std::async(std::launch::async,
                        [src, dst, start, end]() {
                            for (size_t k = start; k < end; ++k)
                                dst[k] = static_cast<double>(src[k]);
                        }));
                }

                for (auto &f : futs) f.get();
                std::sort(&out[0], &out[0] + out.size());
                return out;
            }

            const std::valarray<double> val;
            double fpp;
            double sum;

            // helper: sequential sum for a range
            static double partial_sum(const std::valarray<double>& v, size_t start, size_t end);

            void Summation();

        public:
            

            // from uint16_t valarray
            Percentil(const std::valarray<uint16_t>& array);
            Percentil(const std::valarray<uint16_t>& array, const double& pp);

            // from int16_t valarray
            Percentil(const std::valarray<int16_t>& array);
            Percentil(const std::valarray<int16_t>& array, const double& pp);

            // from uint32_t valarray
            Percentil(const std::valarray<uint32_t>& array);
            Percentil(const std::valarray<uint32_t>& array, const double& pp);

            // from int32_t valarray
            Percentil(const std::valarray<int32_t>& array);
            Percentil(const std::valarray<int32_t>& array, const double& pp);

            // from uint64_t valarray
            Percentil(const std::valarray<uint64_t>& array);
            Percentil(const std::valarray<uint64_t>& array, const double& pp);

            // from int64_t valarray
            Percentil(const std::valarray<int64_t>& array);
            Percentil(const std::valarray<int64_t>& array, const double& pp);

            // from float valarray
            Percentil(const std::valarray<float>& array);
            Percentil(const std::valarray<float>& array, const double& pp);

            // from double valarray
            Percentil(const std::valarray<double>& array);
            Percentil(const std::valarray<double>& array, const double& pp);

            // from uint16_t vector
            Percentil(const std::vector<uint16_t>& array);
            Percentil(const std::vector<uint16_t>& array, const double& pp);

            // from int16_t vector
            Percentil(const std::vector<int16_t>& array);
            Percentil(const std::vector<int16_t>& array, const double& pp);

            // from uint32_t vector
            Percentil(const std::vector<uint32_t>& array);
            Percentil(const std::vector<uint32_t>& array, const double& pp);

            // from int32_t vector
            Percentil(const std::vector<int32_t>& array);
            Percentil(const std::vector<int32_t>& array, const double& pp);

            // from uint64_t vector
            Percentil(const std::vector<uint64_t>& array);
            Percentil(const std::vector<uint64_t>& array, const double& pp);

            // from int64_t vector
            Percentil(const std::vector<int64_t>& array);
            Percentil(const std::vector<int64_t>& array, const double& pp);

            // from float vector
            Percentil(const std::vector<float>& array);
            Percentil(const std::vector<float>& array, const double& pp);

            // from double vector
            Percentil(const std::vector<double>& array);
            Percentil(const std::vector<double>& array, const double& pp);

            // proper move constructor
            Percentil(const Percentil& other) noexcept;

            ~Percentil();

            void SetPercentil(const double& pp);

            double operator()(const std::vector<double>& param) const;

            double Eval(double th) const;

            virtual double Up() const;
        };
    }
}

#endif