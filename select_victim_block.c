#include <stddef.h>
#include <float.h>

/* Represents the metadata required to evaluate a GC candidate block. */
typedef struct {
    unsigned int id;
    unsigned int valid_page_count;
    unsigned int erase_count;
} Block;

/* Computes the GC cost for a candidate based on valid pages and wear leveling. */
static double compute_cost(const Block *block, unsigned int erase_count_max, double wl_weight) {
    /* Normalize the erase counter to determine how close the block is to endurance limits. */
    const double wlf = (block->erase_count >= erase_count_max || erase_count_max == 0U)
        ? 1.0
        : (double)block->erase_count / (double)erase_count_max;

    const double remaining_wear = 1.0 - wlf;
    /* Favor blocks with fewer valid pages while avoiding division by zero. */
    const double cost_term1 = (remaining_wear <= 0.0)
        ? DBL_MAX
        : (double)block->valid_page_count / remaining_wear;

    const double wl_penalty = wl_weight * (double)block->erase_count;
    return cost_term1 + wl_penalty;
}

const Block *select_victim_block(const Block *blocks,
                                 size_t block_count,
                                 unsigned int erase_count_max,
                                 double wl_weight,
                                 double current_pe_avg) {
    (void)current_pe_avg; /* Included for API parity with firmware pseudocode. */

    if (blocks == NULL || block_count == 0U) {
        return NULL;
    }

    const Block *best_block = NULL;
    double min_cost = DBL_MAX;

    for (size_t i = 0U; i < block_count; ++i) {
        const Block *candidate = &blocks[i];
        /* Evaluate the current candidate and keep whichever block yields the lowest cost. */
        const double total_cost = compute_cost(candidate, erase_count_max, wl_weight);
        if (total_cost < min_cost) {
            min_cost = total_cost;
            best_block = candidate;
        }
    }

    return best_block;
}

#ifdef SELECT_VICTIM_BLOCK_DEMO
#include <stdio.h>

int main(void) {
    const Block candidates[] = {
        { .id = 1U, .valid_page_count = 20U, .erase_count = 800U },
        { .id = 2U, .valid_page_count = 10U, .erase_count = 600U },
        { .id = 3U, .valid_page_count = 25U, .erase_count = 400U },
    };

    const unsigned int erase_count_max = 3000U;
    const double wl_weight = 0.05;
    const double current_pe_avg = 500.0;

    const Block *victim = select_victim_block(candidates,
                                              sizeof(candidates) / sizeof(candidates[0]),
                                              erase_count_max,
                                              wl_weight,
                                              current_pe_avg);

    if (victim != NULL) {
        printf("Selected block id %u with %u valid pages and erase count %u\n",
               victim->id,
               victim->valid_page_count,
               victim->erase_count);
    } else {
        printf("No candidate block selected\n");
    }

    return 0;
}
#endif
