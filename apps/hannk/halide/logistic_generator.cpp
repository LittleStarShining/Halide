#include "Halide.h"
#include "common_halide.h"

using namespace Halide;
using namespace Halide::ConciseCasts;

namespace hannk {

// Approximate log2(1 + 2^(x/2^log2_precision))*2^log2_precision_result
Expr approx_log2p1_exp2(Expr x, Expr log2_precision_x, int log2_precision_result) {
    const int log2_p = 8;
    const int p = 1 << log2_p;
    Expr one_plus_exp2_x = p + approx_exp2(x, log2_precision_x, log2_p);

    // If we compute the log2 of the squared value, we can get a bit more precision.
    // This will overflow if one_plus_exp2_x is greater than 256, but this case
    // is not used below.
    Expr raw = approx_log2(pow(one_plus_exp2_x, 2), log2_precision_result - 1);

    // Since we computed log2(x*p) = log2(x) + log2(p), subtract log2(p) now.
    raw = raw - (log2_p << log2_precision_result);

    // For large x, the intermediate overflows. But log2(1 + 2^x) when x is large is just x.
    Expr line = rounding_shift_right(x, log2_precision_x - log2_precision_result);
    Expr threshold = (14 - log2_p) << log2_precision_x;
    return select(x < threshold, raw, line);
}

class Logistic : public Generator<Logistic> {
public:
    Input<Buffer<uint8_t>> input_{"input", 1};

    Input<uint8_t> input_zero_{"input_zero"};
    Input<int32_t> input_multiplier_{"input_multiplier"};
    Input<uint32_t> input_shift_{"input_shift"};

    Output<Buffer<uint8_t>> output_{"output", 1};

    void generate() {
        // The algorithm.
        Var x("x");

        Expr input = i32(i16(input_(x)) - i16(input_zero_)) << 22;
        input = multiply_2x_high(input, input_multiplier_);

        //   256/(1 + 2^input)
        // = 256*2^(-log2(1 + 2^input))
        const int log2_precision = 15;
        Expr log2_inv_logistic = approx_log2p1_exp2(-input, input_shift_, log2_precision);
        Expr logistic = approx_exp2(-log2_inv_logistic, log2_precision, 8);

        output_(x) = u8_sat(logistic);

        // Schedule.
        const int vector_size = natural_vector_size<uint8_t>();

        output_.vectorize(x, vector_size, TailStrategy::Predicate);
    }
};

}  // namespace hannk

HALIDE_REGISTER_GENERATOR(hannk::Logistic, Logistic)