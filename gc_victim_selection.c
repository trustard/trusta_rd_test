#include <stdio.h>
#include <stdlib.h>
#include <float.h>

// Structure to represent a block
typedef struct {
    int valid_page_count;    // Number of valid pages in the block
    int p_e_cycle_count;     // Program/Erase cycle count for the block
    int block_id;            // Block identifier
} Block;

// Function to select the best victim block for garbage collection
Block* select_victim_block(Block* candidate_blocks, int num_blocks, 
                         int erase_count_max, float wl_weight, float current_p_e_avg) {
    float min_cost = FLT_MAX;
    Block* best_victim_block = NULL;

    // Iterate through each candidate block
    for (int i = 0; i < num_blocks; i++) {
        Block* current_block = &candidate_blocks[i];
        
        // 1. Retrieve Block Status
        int n_valid = current_block->valid_page_count;
        int erase_count = current_block->p_e_cycle_count;

        // 2. Calculate Wear Leveling Factor (WLF)
        float wlf;
        if (erase_count >= erase_count_max) {
            wlf = 1.0f;  // Avoid division by zero/overflow if maximum is reached
        } else {
            wlf = (float)erase_count / erase_count_max;
        }

        // 3. Calculate Cost-Benefit Function C(B)
        float cost_term1;
        if ((1.0f - wlf) == 0) {
            cost_term1 = FLT_MAX;  // Should not happen if EraseCount_Max is defined correctly
        } else {
            cost_term1 = (float)n_valid / (1.0f - wlf);  // Prioritizes low N_Valid (low WA)
        }

        float wl_penalty = wl_weight * erase_count;  // Penalizes high P/E counts
        float total_cost = cost_term1 + wl_penalty;

        // 4. Update Minimum Cost Block
        if (total_cost < min_cost) {
            min_cost = total_cost;
            best_victim_block = current_block;
        }
    }

    return best_victim_block;
}

// Example usage
int main() {
    // Example parameters
    const int ERASE_COUNT_MAX = 3000;  // for TLC NAND
    const float WL_WEIGHT = 0.05f;
    const float CURRENT_P_E_AVG = 500.0f;

    // Create some example blocks
    Block candidate_blocks[] = {
        {.valid_page_count = 10, .p_e_cycle_count = 1000, .block_id = 1},
        {.valid_page_count = 5,  .p_e_cycle_count = 2000, .block_id = 2},
        {.valid_page_count = 15, .p_e_cycle_count = 500,  .block_id = 3},
        {.valid_page_count = 8,  .p_e_cycle_count = 1500, .block_id = 4}
    };
    int num_blocks = sizeof(candidate_blocks) / sizeof(candidate_blocks[0]);

    // Select victim block
    Block* victim = select_victim_block(candidate_blocks, num_blocks, 
                                      ERASE_COUNT_MAX, WL_WEIGHT, CURRENT_P_E_AVG);

    // Print result
    if (victim != NULL) {
        printf("Selected victim block:\n");
        printf("Block ID: %d\n", victim->block_id);
        printf("Valid Pages: %d\n", victim->valid_page_count);
        printf("P/E Cycles: %d\n", victim->p_e_cycle_count);
    } else {
        printf("No suitable victim block found.\n");
    }

    return 0;
}