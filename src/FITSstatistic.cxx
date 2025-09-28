#include <FITSstatistic.h>
#include <cmath>
#include <stdexcept>

namespace DSL
{
    namespace stat
    {

        std::size_t Percentil::lower_bound(const double& v) const
        {
            std::size_t left = 0;
            std::size_t right = val.size();
            
            while (left < right)
            {
                std::size_t mid = left + (right - left) / 2;
                if (val[mid] < v)
                    left = mid + 1;
                else
                    right = mid;
            }
            return left; // index of first element >= v (may be val.size())
        }

        // helper: sequential sum for a range
        double Percentil::partial_sum(const std::valarray<double>& v, size_t start, size_t end)
        {
            std::valarray<double> s = v[std::slice(start, end - start,1)];
            return s.sum();
        }

        void Percentil::Summation()
        {
            // parallel summation
            unsigned int hw_threads = std::thread::hardware_concurrency();
            unsigned int nthreads = (hw_threads == 0) ? 1 : hw_threads;
            if (static_cast<size_t>(nthreads) > val.size()) nthreads = static_cast<unsigned int>(val.size());
            
            if (nthreads <= 1 or val.size()/nthreads < 1)
            {
                sum = val.sum();
                return;
            }   

            const size_t chunk = (val.size() + nthreads - 1) / nthreads;
            std::vector< std::future<double> > futs;
            futs.reserve(nthreads);
            for (unsigned int ti = 0; ti < nthreads; ++ti)
            {
                const size_t start = ti * chunk;
                const size_t end = std::min(val.size(), start + chunk);
                if (start >= end) continue;

                // capture pointer and bounds by value
                futs.emplace_back(std::async(std::launch::async,
                    [this, start, end]() {
                        return Percentil::partial_sum(this->val, start, end);
                    }));
            }
            sum = 0.0;
            for (auto &f : futs) sum += f.get();
        }

        // from uint16_t valarray
        Percentil::Percentil(const std::valarray<uint16_t>& array): val(Percentil::to_valarray<uint16_t>(array)), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::valarray<uint16_t>& array, const double& pp): val(Percentil::to_valarray<uint16_t>(array)), fpp(pp), sum(0.0)
        {
            Summation();
        }

        // from int16_t valarray
        Percentil::Percentil(const std::valarray<int16_t>& array): val(Percentil::to_valarray<int16_t>(array)), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::valarray<int16_t>& array, const double& pp): val(Percentil::to_valarray<int16_t>(array)), fpp(pp), sum(0.0)
        {
            Summation();
        }

        // from uint32_t valarray
        Percentil::Percentil(const std::valarray<uint32_t>& array): val(Percentil::to_valarray<uint32_t>(array)), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::valarray<uint32_t>& array, const double& pp): val(Percentil::to_valarray<uint32_t>(array)), fpp(pp), sum(0.0)
        {
            Summation();
        }

        // from int32_t valarray
        Percentil::Percentil(const std::valarray<int32_t>& array): val(Percentil::to_valarray<int32_t>(array)), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::valarray<int32_t>& array, const double& pp): val(Percentil::to_valarray<int32_t>(array)), fpp(pp), sum(0.0)
        {
            Summation();
        }

        // from uint64_t valarray
        Percentil::Percentil(const std::valarray<uint64_t>& array): val(Percentil::to_valarray<uint64_t>(array)), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::valarray<uint64_t>& array, const double& pp): val(Percentil::to_valarray<uint64_t>(array)), fpp(pp), sum(0.0)
        {
            Summation();
        }

        // from int64_t valarray
        Percentil::Percentil(const std::valarray<int64_t>& array): val(Percentil::to_valarray<int64_t>(array)), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::valarray<int64_t>& array, const double& pp): val(Percentil::to_valarray<int64_t>(array)), fpp(pp), sum(0.0)
        {
            Summation();
        }

        // from float valarray
        Percentil::Percentil(const std::valarray<float>& array): val(Percentil::to_valarray<float>(array)), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::valarray<float>& array, const double& pp): val(Percentil::to_valarray<float>(array)), fpp(pp), sum(0.0)
        {
            Summation();
        }

        // from double valarray
        Percentil::Percentil(const std::valarray<double>& array): val([&](){std::valarray<double> tmp = array;
                                                                            if (tmp.size() > 1)
                                                                                std::sort(&tmp[0], &tmp[0] + tmp.size());
                                                                            return tmp;}()), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::valarray<double>& array, const double& pp): val([&](){std::valarray<double> tmp = array;
                                                                            if (tmp.size() > 1)
                                                                                std::sort(&tmp[0], &tmp[0] + tmp.size());
                                                                            return tmp;}()), fpp(pp), sum(0.0)
        {
            Summation();
        }

        // from uint16_t vector
        Percentil::Percentil(const std::vector<uint16_t>& array): val(Percentil::to_valarray<uint16_t>(array)), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::vector<uint16_t>& array, const double& pp): val(Percentil::to_valarray<uint16_t>(array)), fpp(pp), sum(0.0)
        {
            Summation();
        }

        // from int16_t vector
        Percentil::Percentil(const std::vector<int16_t>& array): val(Percentil::to_valarray<int16_t>(array)), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::vector<int16_t>& array, const double& pp): val(Percentil::to_valarray<int16_t>(array)), fpp(pp), sum(0.0)
        {
            Summation();
        }
        
        // from uint32_t vector
        Percentil::Percentil(const std::vector<uint32_t>& array): val(Percentil::to_valarray<uint32_t>(array)), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::vector<uint32_t>& array, const double& pp): val(Percentil::to_valarray<uint32_t>(array)), fpp(pp), sum(0.0)
        {
            Summation();
        }

        // from int32_t vector
        Percentil::Percentil(const std::vector<int32_t>& array): val(Percentil::to_valarray<int32_t>(array)), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::vector<int32_t>& array, const double& pp): val(Percentil::to_valarray<int32_t>(array)), fpp(pp), sum(0.0)
        {
            Summation();
        }

        // from uint64_t vector
        Percentil::Percentil(const std::vector<uint64_t>& array): val(Percentil::to_valarray<uint64_t>(array)), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::vector<uint64_t>& array, const double& pp): val(Percentil::to_valarray<uint64_t>(array)), fpp(pp), sum(0.0)
        {
            Summation();
        }

        // from int64_t vector
        Percentil::Percentil(const std::vector<int64_t>& array): val(Percentil::to_valarray<int64_t>(array)), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::vector<int64_t>& array, const double& pp): val(Percentil::to_valarray<int64_t>(array)), fpp(pp), sum(0.0)
        {
            Summation();
        }

        // from float vector
        Percentil::Percentil(const std::vector<float>& array): val(Percentil::to_valarray<float>(array)), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::vector<float>& array, const double& pp): val(Percentil::to_valarray<float>(array)), fpp(pp), sum(0.0)
        {
            Summation();
        }

        // from double vector
        Percentil::Percentil(const std::vector<double>& array): val([&](){std::valarray<double> tmp(array.data(), array.size());
                                                                          if (tmp.size() > 1)
                                                                            std::sort(&tmp[0], &tmp[0] + tmp.size());
                                                                        return tmp;}()), fpp(0.5), sum(0.0)
        {
            Summation();
        }

        Percentil::Percentil(const std::vector<double>& array, const double& pp): val([&](){std::valarray<double> tmp(array.data(), array.size());
                                                                                        if (tmp.size() > 1)
                                                                                            std::sort(&tmp[0], &tmp[0] + tmp.size());
                                                                                        return tmp;}()), fpp(pp), sum(0.0)
        {
            Summation();
        }

        // proper move constructor
        Percentil::Percentil(const Percentil& other) noexcept : val(other.val), fpp(other.fpp), sum(other.sum)
        {
            Summation();
        }

        Percentil::~Percentil() {}

        void Percentil::SetPercentil(const double& pp) { fpp *=0; fpp += pp; }

        double Percentil::operator()(const std::vector<double>& param) const
        {
            return pow(  Eval(param[0]) - fpp, 2.) ;
        }

        double Percentil::Eval(double th) const
        {
            const std::size_t n = val.size();
            if (n == 0)
                throw std::invalid_argument("Empty data array");

            if (val[0] == val[n - 1])
            {
                if (th == val[0])
                    return 0.5;
                return (th < val[0]) ? 0.0 : 1.0;
            }
        
            // Below min
            if (th <= val[0])
                return 0.0;
        
            // Above max
            if (th >= val[n - 1])
                return 1.0;
        
            // Find position
            std::size_t j = lower_bound(th);
        
            // Exact match? Handle ties by averaging index range
            if (j < n && val[j] == th)
            {
                std::size_t start = j;
                while (start > 0 && val[start - 1] == th) --start;
                
                std::size_t end = j;
                while (end + 1 < n && val[end + 1] == th) ++end;
                double pos = 0.5 * (start + end);
            
                return pos / (n - 1);
            }
        
            // Between sorted[j-1] and sorted[j]
            double x0 = val[j - 1];
            double x1 = val[j];
            double frac = (th - x0) / (x1 - x0);
            double pos = (j - 1) + frac;
        
            return pos / (n - 1);
        }

        double Percentil::Up() const { return 4; }

    } // namespace stat
} // namespace DSL
