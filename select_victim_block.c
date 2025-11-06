#include <stddef.h>
#include <float.h>

typedef struct {
    unsigned int valid_page_count;
    unsigned int p_e_cycle_count;
} Block;

const Block *select_victim_block(const Block *candidate_blocks,
                                 size_t candidate_count,
                                 unsigned int erase_count_max,
                                 double wl_weight,
                                 double current_p_e_avg) {
    if (candidate_blocks == NULL || candidate_count == 0) {
        return NULL;
    }

    (void)current_p_e_avg; // Reserved for future heuristics that use fleet-wide averages.

    const Block *best_block = NULL;
    double min_cost = DBL_MAX;

    for (size_t i = 0; i < candidate_count; ++i) {
        const Block *block = &candidate_blocks[i];
        double n_valid = (double)block->valid_page_count;
        double erase_count = (double)block->p_e_cycle_count;

        double normalized_max = erase_count_max > 0 ? (double)erase_count_max : 1.0;
        double wlf = normalized_max > 0.0 ? (erase_count / normalized_max) : 1.0;
        if (wlf > 1.0) {
            wlf = 1.0;
        }

        double cost_term1 = DBL_MAX;
        double wear_level_factor = 1.0 - wlf;
        if (wear_level_factor > 0.0) {
            cost_term1 = n_valid / wear_level_factor;
        }

        double wl_penalty = wl_weight * erase_count;
        double total_cost = cost_term1 + wl_penalty;

        if (total_cost < min_cost) {
            min_cost = total_cost;
            best_block = block;
        }
    }

    return best_block;
}

#ifdef SELECT_VICTIM_BLOCK_DEMO
#include <stdio.h>

int main(void) {
    Block candidates[] = {
        { .valid_page_count = 24, .p_e_cycle_count = 900 },
        { .valid_page_count = 12, .p_e_cycle_count = 650 },
        { .valid_page_count = 16, .p_e_cycle_count = 1200 }
    };

    const Block *result = select_victim_block(candidates,
                                              sizeof(candidates) / sizeof(candidates[0]),
                                              3000,
                                              0.05,
                                              750.0);

    if (result != NULL) {
        size_t index = (size_t)(result - candidates);
        printf("Victim block index: %zu\n", index);
        printf("Valid pages: %u\n", result->valid_page_count);
        printf("P/E cycles: %u\n", result->p_e_cycle_count);
    } else {
        printf("No victim block selected.\n");
    }

    return 0;
}
#endif
