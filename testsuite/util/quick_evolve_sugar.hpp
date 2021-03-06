/*!
 * \file quick_evolve_sugar.hpp
 * \brief Helper functions for unit/integration testing
 * \ingroup unit
 */
#ifndef FWDPP_TESTSUITE_UTIL_QUICK_EVOLVE_SUGAR_HPP
#define FWDPP_TESTSUITE_UTIL_QUICK_EVOLVE_SUGAR_HPP

#include <fwdpp/recombination.hpp>
#include <fwdpp/fitness_models.hpp>
#include <fwdpp/sample_diploid.hpp>
#include <fwdpp/util.hpp>
#include <fwdpp/sugar/infsites.hpp>
#include <fwdpp/sugar/GSLrng_t.hpp>
#include <testsuite/util/migpop.hpp>
#include <testsuite/fixtures/sugar_fixtures.hpp>

template <typename singlepop_object_t>
void
simulate_singlepop(singlepop_object_t &pop, const unsigned simlen = 10)
/*!
  \brief Quick function for evolving a single-deme simulation
  \ingroup testing
  \note Do NOT call this function repeatedly on the same population.
 */
{
    KTfwd::GSLrng_t<KTfwd::GSL_RNG_TAUS2> rng(0u);

    for (unsigned generation = 0; generation < simlen; ++generation)
        {
            double wbar = KTfwd::sample_diploid(
                rng.get(), pop.gametes, pop.diploids, pop.mutations,
                pop.mcounts, 1000, 0.005,
                std::bind(KTfwd::infsites(), std::placeholders::_1,
                          std::placeholders::_2, rng.get(),
                          std::ref(pop.mut_lookup), generation, 0.0025, 0.0025,
                          [&rng]() { return gsl_rng_uniform(rng.get()); },
                          []() { return -0.01; }, []() { return 1.; }),
                std::bind(KTfwd::poisson_xover(), rng.get(), 0.005, 0., 1.,
                          std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3),
                std::bind(KTfwd::multiplicative_diploid(),
                          std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3, 2),
                pop.neutral, pop.selected);
            KTfwd::update_mutations(pop.mutations, pop.fixations,
                                    pop.fixation_times, pop.mut_lookup,
                                    pop.mcounts, generation, 2 * pop.N);
        }
}

template <typename singlepop_object_t, typename rng_type>
unsigned
simulate_singlepop(singlepop_object_t &pop, const rng_type &rng,
                   const unsigned generation, const unsigned simlen)
/*!
  \brief Quick function for evolving a single-deme simulation
  \ingroup testing
  \note this version CAN be used on the same population object
 */
{
    unsigned g = generation;
    for (; g < generation + simlen; ++g)
        {
            double wbar = KTfwd::sample_diploid(
                rng.get(), pop.gametes, pop.diploids, pop.mutations,
                pop.mcounts, 1000, 0.005,
                std::bind(KTfwd::infsites(), std::placeholders::_1,
                          std::placeholders::_2, rng.get(),
                          std::ref(pop.mut_lookup), g, 0.0025, 0.0025,
                          [&rng]() { return gsl_rng_uniform(rng.get()); },
                          []() { return -0.01; }, []() { return 1.; }),
                std::bind(KTfwd::poisson_xover(), rng.get(), 0.005, 0., 1.,
                          std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3),
                std::bind(KTfwd::multiplicative_diploid(),
                          std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3, 2),
                pop.neutral, pop.selected);
            KTfwd::update_mutations(pop.mutations, pop.fixations,
                                    pop.fixation_times, pop.mut_lookup,
                                    pop.mcounts, g, 2 * pop.N);
        }
    return g + simlen;
}

// Fitness function
struct multilocus_additive
{
    using result_type = double;
    inline double
    operator()(
        const multiloc_popgenmut_fixture::poptype::dipvector_t::value_type
            &diploid,
        const multiloc_popgenmut_fixture::poptype::gcont_t &gametes,
        const multiloc_popgenmut_fixture::poptype::mcont_t &mutations) const
    {
        using dip_t = multiloc_popgenmut_fixture::poptype::dipvector_t::
            value_type::value_type;
        return std::max(
            0., 1. + std::accumulate(diploid.begin(), diploid.end(), 0.,
                                     [&gametes, &mutations](const double d,
                                                            const dip_t &dip) {
                                         return d + KTfwd::additive_diploid()(
                                                        gametes[dip.first],
                                                        gametes[dip.second],
                                                        mutations)
                                                - 1.;
                                     }));
    }
};

template <typename rng_type>
inline unsigned
simulate_mlocuspop(multiloc_popgenmut_fixture::poptype &pop,
                   const rng_type &rng, const unsigned generation,
                   const unsigned simlen)
/*!
  \brief Quick function for evolving a multilocus deme simulation
  \ingroup testing
  \note this version CAN be used on the same population object
 */
{
    unsigned g = generation;
    std::vector<std::function<std::size_t(
        std::queue<std::size_t> &,
        multiloc_popgenmut_fixture::poptype::mcont_t &)>>
        mutmodels{
            // Locus 0: positions Uniform [0,1)
            std::bind(KTfwd::infsites(), std::placeholders::_1,
                      std::placeholders::_2, rng.get(),
                      std::ref(pop.mut_lookup), &g, 0.0025, 0.0025,
                      [&rng]() { return gsl_rng_uniform(rng.get()); },
                      []() { return -0.01; }, []() { return 1.; }),
            // Locus 1: positions Uniform [1,2)
            std::bind(KTfwd::infsites(), std::placeholders::_1,
                      std::placeholders::_2, rng.get(),
                      std::ref(pop.mut_lookup), &g, 0.0025, 0.0025,
                      [&rng]() { return gsl_ran_flat(rng.get(), 1., 2.); },
                      []() { return -0.01; }, []() { return 1.; }),
            // Locus 2: positions Uniform [2,3)
            std::bind(KTfwd::infsites(), std::placeholders::_1,
                      std::placeholders::_2, rng.get(),
                      std::ref(pop.mut_lookup), &g, 0.0025, 0.0025,
                      [&rng]() { return gsl_ran_flat(rng.get(), 2., 3.); },
                      []() { return -0.01; }, []() { return 1.; }),
            // Locus 3: positions Uniform [3,4)
            std::bind(KTfwd::infsites(), std::placeholders::_1,
                      std::placeholders::_2, rng.get(),
                      std::ref(pop.mut_lookup), &g, 0.0025, 0.0025,
                      [&rng]() { return gsl_ran_flat(rng.get(), 3., 4.); },
                      []() { return -0.01; }, []() { return 1.; })
        };

    // Within-locus recombination models for 4 loci
    std::vector<std::function<std::vector<double>(
        const multiloc_popgenmut_fixture::poptype::gamete_t &,
        const multiloc_popgenmut_fixture::poptype::gamete_t &,
        const multiloc_popgenmut_fixture::poptype::mcont_t &)>>
        recmodels{ std::bind(KTfwd::poisson_xover(), rng.get(), 0.005, 0., 1.,
                             std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3),
                   std::bind(KTfwd::poisson_xover(), rng.get(), 0.005, 1., 2.,
                             std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3),
                   std::bind(KTfwd::poisson_xover(), rng.get(), 0.005, 2., 3.,
                             std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3),
                   std::bind(KTfwd::poisson_xover(), rng.get(), 0.005, 3., 4.,
                             std::placeholders::_1, std::placeholders::_2,
                             std::placeholders::_3) };

    // Equal mutation and rec. rates between loci
    std::vector<double> mu(4, 0.005), rbw(3, 0.005);
    for (; g < generation + simlen; ++g)
        {
            double wbar = KTfwd::sample_diploid(
                rng.get(), pop.gametes, pop.diploids, pop.mutations,
                pop.mcounts, 1000, &mu[0], mutmodels, recmodels, &rbw[0],
                [](const gsl_rng *__r, const double __d) {
                    return gsl_ran_binomial(__r, __d, 1);
                },
                std::bind(multilocus_additive(), std::placeholders::_1,
                          std::placeholders::_2, std::placeholders::_3),
                pop.neutral, pop.selected);
            assert(check_sum(pop.gametes, 8000));
            KTfwd::update_mutations(pop.mutations, pop.fixations,
                                    pop.fixation_times, pop.mut_lookup,
                                    pop.mcounts, generation, 2000);
        }
    return g + simlen;
}

template <typename metapop_object>
void
simulate_metapop(metapop_object &pop, const unsigned simlen = 10)
{
    // Evolve for 10 generations
    std::vector<std::function<double(
        const typename metapop_object::gamete_t &,
        const typename metapop_object::gamete_t &,
        const typename metapop_object::mcont_t &)>>
        fitness_funcs(2,
                      std::bind(KTfwd::multiplicative_diploid(),
                                std::placeholders::_1, std::placeholders::_2,
                                std::placeholders::_3, 2.));
    KTfwd::GSLrng_t<KTfwd::GSL_RNG_TAUS2> rng(0u);
    for (unsigned generation = 0; generation < simlen; ++generation)
        {
            std::vector<double> wbar = KTfwd::sample_diploid(
                rng.get(), pop.gametes, pop.diploids, pop.mutations,
                pop.mcounts, &pop.Ns[0], 0.005,
                std::bind(KTfwd::infsites(), std::placeholders::_1,
                          std::placeholders::_2, rng.get(),
                          std::ref(pop.mut_lookup), generation, 0.005, 0.,
                          [&rng]() { return gsl_rng_uniform(rng.get()); },
                          []() { return 0.; }, []() { return 0.; }),
                std::bind(KTfwd::poisson_xover(), rng.get(), 0.005, 0., 1.,
                          std::placeholders::_1, std::placeholders::_2,
                          std::placeholders::_3),
                fitness_funcs,
                std::bind(migpop, std::placeholders::_1, rng.get(), 0.001),
                pop.neutral, pop.selected);
            KTfwd::update_mutations(pop.mutations, pop.fixations,
                                    pop.fixation_times, pop.mut_lookup,
                                    pop.mcounts, generation, 4000);
        }
}

#endif
