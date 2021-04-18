
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <time.h>

#include <limits>

struct RoundingType {
    enum {
        RoundDown,
        RoundUp,
        HalfAdjust
    };
};

uint32_t next_random32()
{
#if (RAND_MAX == 0x7FFF)
    return (((rand() & 0x7FFF) << 30) | ((rand() & 0x7FFF) << 15) | (rand() & 0x7FFF));
#else
    return rand();
#endif
}

//
// See: https://www.zhihu.com/question/29971598
//
double next_random_box_muller(double mu, double sigma)
{
	const double epsilon = std::numeric_limits<double>::min();
	const double two_pi = 2.0 * 3.14159265358979323846;

	static double z0, z1;
	static bool generate;
	generate = !generate;

	if (!generate) {
	   return (z1 * sigma + mu);
    }

	double u1, u2;
	do {
	   u1 = rand() * (1.0 / RAND_MAX);
	   u2 = rand() * (1.0 / RAND_MAX);
	} while (u1 <= epsilon);

	z0 = sqrt(-2.0 * log(u1)) * cos(two_pi * u2);
	z1 = sqrt(-2.0 * log(u1)) * sin(two_pi * u2);
	return (z0 * sigma + mu);
}

int32_t next_random_i32(int32_t min_num, int32_t max_num)
{
    if (min_num < max_num)
        return (min_num + (next_random32() % uint32_t(max_num - min_num + 1)));
    else if (min_num > max_num)
        return (max_num + (next_random32() % uint32_t(min_num - max_num + 1)));
    else
        return min_num;
}

int64_t next_random_i64(int64_t min_num, int64_t max_num)
{
    if (min_num < max_num)
        return (min_num + (next_random32() % uint32_t(max_num - min_num + 1)));
    else if (min_num > max_num)
        return (max_num + (next_random32() % uint32_t(min_num - max_num + 1)));
    else
        return min_num;
}

double normal_dist_next_random()
{
    const double l_limit = -3.0, r_limit = 3.0;
    double randomf = next_random_box_muller(0.0, 1.0);
    if (randomf > r_limit)
        randomf = r_limit;
    else if (randomf < l_limit)
        randomf = l_limit;

    // normalize to [-1, 1]
    randomf /= r_limit;

    // translation to [0, 1]
    randomf = (randomf + 1.0) / 2.0;
    return randomf;
}

int32_t normal_dist_random_i32(int32_t min_num, int32_t max_num)
{
    double randomf = normal_dist_next_random();

    if (min_num < max_num)
        return (min_num + int32_t(randomf * (max_num - min_num)));
    else if (min_num > max_num)
        return (max_num + int32_t(randomf * (min_num - max_num)));
    else
        return min_num;
}

int64_t normal_dist_random_i64(int64_t min_num, int64_t max_num)
{
    double randomf = normal_dist_next_random();

    if (min_num < max_num)
        return (min_num + int64_t(randomf * (max_num - min_num)));
    else if (min_num > max_num)
        return (max_num + int64_t(randomf * (min_num - max_num)));
    else
        return min_num;
}

double normal_dist_random_f(double minimun, double maximum)
{
    double randomf = normal_dist_next_random();

    if (minimun < maximum)
        return (minimun + randomf * (maximum - minimun));
    else if (minimun > maximum)
        return (maximum + randomf * (minimun - maximum));
    else
        return minimun;
}

double round_price(double price, double precision = 100.0, int round_type = RoundingType::HalfAdjust)
{
    if (round_type == RoundingType::RoundDown)
        return floor(price * precision) / precision;
    else if (round_type == RoundingType::RoundUp)
        return ceil(price * precision) / precision;
    else
        return floor(price * precision + 0.5) / precision;
}

struct GoodsInvoice
{
    bool        auto_release;
    size_t      count;
    double *    prices;
    size_t *    amounts;
    double *    moneys;

    GoodsInvoice() : auto_release(false), count(0), prices(nullptr),
                     amounts(nullptr), moneys(nullptr) {
    }

    GoodsInvoice(const GoodsInvoice & other) {
        this->copy(other);
    }

    ~GoodsInvoice() {
        this->destroy();
    }

    void destroy() {
        if (auto_release) {
            if (this->prices) {
                delete[] this->prices;
                this->prices = nullptr;
            }
            if (this->amounts) {
                delete[] this->amounts;
                this->amounts = nullptr;
            }
            if (this->moneys) {
                delete[] this->moneys;
                this->moneys = nullptr;
            }
        }
    }

    GoodsInvoice & operator = (const GoodsInvoice & rhs) {
        if (&rhs != this) {
            this->destroy();
            this->copy(rhs);
        }
        return *this;
    }

    void copy(const GoodsInvoice & invoice) {
        if (!invoice.auto_release) {
            this->auto_release  = invoice.auto_release;
            this->count         = invoice.count;
            this->prices        = invoice.prices;
            this->amounts       = invoice.amounts;
            this->moneys        = invoice.moneys;
        }
        else {
            this->clone(invoice.count, invoice.prices, invoice.amounts, invoice.moneys);
        }
    }

    bool clone(size_t goods_count, double goods_price[],
               size_t goods_amount[], double goods_money[] = nullptr) {
        auto_release = true;
        this->count = goods_count;

        double * new_goods_price = new double[goods_count];
        if (new_goods_price == nullptr) {
            return false;
        }

        for (size_t i = 0; i < goods_count; i++) {
            new_goods_price[i] = goods_price[i];
        }
        this->prices = new_goods_price;

        size_t * new_goods_amount = new size_t[goods_count];
        if (new_goods_amount == nullptr) {
            return false;
        }

        for (size_t i = 0; i < goods_count; i++) {
            new_goods_amount[i] = goods_amount[i];
        }
        this->amounts = new_goods_amount;

        double * new_goods_money = new double[goods_count];
        if (new_goods_money == nullptr) {
            return false;
        }

        if (goods_money == nullptr) {
            for (size_t i = 0; i < goods_count; i++) {
                new_goods_money[i] = 0.0;
            }
        }
        else {
            for (size_t i = 0; i < goods_count; i++) {
                new_goods_money[i] = goods_money[i];
            }
        }
        this->moneys = new_goods_money;

        return true;
    }

    void set_price_amount(size_t goods_count, double goods_price[],
                          size_t goods_amount[], double goods_money[] = nullptr) {
        this->destroy();

        this->auto_release  = false;
        this->count         = goods_count;
        this->prices        = goods_price;
        this->amounts       = goods_amount;
        this->moneys        = goods_money;
    }

    bool create_price_amount(size_t goods_count, double goods_price[],
                             size_t goods_amount[], double goods_money[] = nullptr) {
        this->destroy();
        bool result = this->clone(goods_count, goods_price, goods_amount, goods_money);
        return result;
    }
};

class InvoiceBalance
{
private:
    double          total_price_;
    double          fluctuation_;

    size_t          goods_count_;

    GoodsInvoice    in_invoice_;
    GoodsInvoice    invoice_;
    GoodsInvoice    best_answer_;

public:
    InvoiceBalance(double total_price, double fluctuation)
        : total_price_(total_price), fluctuation_(fluctuation), goods_count_(0) {
    }

    virtual ~InvoiceBalance() {
    }

    bool set_price_amount(size_t goods_count, double goods_price[], size_t goods_amount[]) {
        this->goods_count_ = goods_count;
        this->in_invoice_.set_price_amount(goods_count, goods_price, goods_amount);
        bool result = this->invoice_.create_price_amount(goods_count, goods_price, goods_amount);
        return result;
    }

    bool normalize() {
        bool result = true;
        this->fluctuation_ = round_price(this->fluctuation_);
        for (size_t i = 0; i < this->in_invoice_.count; i++) {
            this->in_invoice_.prices[i] = round_price(this->in_invoice_.prices[i]);
            if (this->in_invoice_.prices[i] < 0.0) {
                result = false;
            }
        }
        return result;
    }

private:
    double calc_total_price(double total_price, size_t goods_count,
                            double goods_price[], size_t goods_amount[],
                            double goods_money[]) {
        double actual_total_price = 0.0;
        for (size_t i = 0; i < goods_count; i++) {
            double money = round_price(goods_price[i] * goods_amount[i]);
            goods_money[i] = money;
            actual_total_price += money;
        }
        return actual_total_price;
    }

    double calc_total_price(double total_price, GoodsInvoice & invoice) {
        return calc_total_price(total_price, invoice.count, invoice.prices,
                                             invoice.amounts, invoice.moneys);
    }

    double calc_min_total_price(size_t idx, double total_price,
                                size_t goods_count, double goods_price[]) {
        double actual_total_price = 0.0;
        for (size_t i = 0; i < goods_count; i++) {
            if (idx == i) continue;
            double money = round_price(goods_price[i] * 1.0);
            actual_total_price += money;
        }
        return actual_total_price;
    }

    int recalc_max_goods_amount(size_t idx) {
        double actual_total_price = calc_total_price(this->total_price_, this->invoice_);
        double min_total_price = calc_min_total_price(idx, this->total_price_,
                                                      this->invoice_.count, this->invoice_.prices);
        return (int)((this->total_price_ - actual_total_price - min_total_price) /
                     (this->invoice_.prices[idx] + this->fluctuation_));
    }

    size_t find_padding_idx(size_t goods_count, size_t goods_amount[]) {
        size_t count = 0;
        size_t padding_idx = size_t(-1);
        for (size_t i = 0; i < goods_count; i++) {
            if (goods_amount[i] == 0.0) {
                if (count == 0)
                    padding_idx = i;
                count++;
            }
        }
        return ((count == 1) ? padding_idx : size_t(-1));
    }

    int adjust_price_and_count(double & price_error,
                               double total_price, double fluctuation,
                               GoodsInvoice & invoice) {
        int result = 0;
        size_t padding_idx = find_padding_idx(invoice.count, invoice.amounts);
        if (padding_idx == size_t(-1)) {
            return -1;
        }
        double actual_total_price = calc_total_price(total_price, invoice);
        if (actual_total_price <= total_price) {
            ptrdiff_t padding_amount = (ptrdiff_t)((total_price - actual_total_price) / invoice.prices[padding_idx]);
            invoice.amounts[padding_idx] = padding_amount;
        }
        else {
            return -1;
        }

        actual_total_price = calc_total_price(total_price, invoice);

        price_error = actual_total_price - total_price;
        return result;
    }

    bool search_price_and_count() {
        bool solvable = false;

        double total_price = this->total_price_;
        double fluctuation = this->fluctuation_;
        size_t goods_count = this->invoice_.count;

        size_t * max_goods_amount = new size_t[goods_count];
        if (max_goods_amount == nullptr) {
            return false;
        }
        for (size_t i = 0; i < goods_count; i++) {
            double remain_total_price = total_price;
            for (size_t j = 0; j < goods_count; j++) {
                if (i != j) {
                    remain_total_price -= this->invoice_.prices[j] + fluctuation;
                }
            }
            max_goods_amount[i] = (size_t)(remain_total_price / (this->invoice_.prices[i] + fluctuation));
        }

        size_t search_cnt = 0;
        double min_price_error = 9999.0;
        double price_error = 9999.0;

        while (price_error != 0.0) {
            for (size_t i = 0; i < goods_count; i++) {
                double price_change = normal_dist_random_f(-this->fluctuation_, this->fluctuation_);
                this->invoice_.prices[i] = round_price(this->in_invoice_.prices[i] + price_change);
            }

            for (size_t i = 0; i < goods_count; i++) {
                this->invoice_.amounts[i] = 0;
            }
            for (size_t i = 0; i < goods_count - 1; i++) {
NEXT_GOODS_AMOUNT:
                size_t idx = next_random_i64(0, goods_count - 1);
                if (this->invoice_.amounts[idx] != 0.0) {
                    goto NEXT_GOODS_AMOUNT;
                }
                int goods_amount_limit = recalc_max_goods_amount(idx);
                this->invoice_.amounts[idx] = normal_dist_random_i32(1, goods_amount_limit);
            }

            int result = adjust_price_and_count(price_error, total_price, fluctuation, this->invoice_);
            if (result == 0) {
                if (price_error < min_price_error) {
                    min_price_error = price_error;
                    this->best_answer_ = this->invoice_;
                }
            }

            search_cnt++;
            if (search_cnt > 100000)
                break;
        }

        return solvable;
    }

public:
    int solve() {
        int result = 0;
        this->normalize();

        bool solvable = search_price_and_count();
        if (solvable) {
            //
        }
        else {
            //
        }
        return result;
    }
};

int main(int argc, char * argv[])
{
    ::srand((unsigned int)::time(NULL));

    double goods_price[3] = {
        212.00,
        172.5,
        226.0
    };

    size_t goods_amount[3] = { 0 };

    InvoiceBalance invoiceBalance(120000.0, 2.0);
    invoiceBalance.set_price_amount(3, goods_price, goods_amount);

    int result = invoiceBalance.solve();
    return result;
}
