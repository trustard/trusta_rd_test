// Input:
//   Candidate_Blocks: List of all erase blocks available for GC
//   EraseCount_Max: Maximum allowed P/E cycles for the NAND flash
//   WL_Weight: Wear Leveling weight factor (W_WL)
//   Current_P_E_Avg: Average P/E cycle count across all blocks
// Output:
//   Best_Victim_Block: The block selected for immediate erasure (GC)
FUNCTION Select_Victim_Block(Candidate_Blocks, EraseCount_Max, WL_Weight, Current_P_E_Avg):
    Min_Cost = INFINITY
    Best_Victim_Block = NULL
    FOR EACH Block IN Candidate_Blocks:
        // 1. Retrieve Block Status
        N_Valid = Block.Valid_Page_Count      // Number of valid pages (data to be copied)
        Erase_Count = Block.P_E_Cycle_Count  // Block's current erase count
        // 2. Calculate Wear Leveling Factor (WLF)
        // WLF is used to prioritize low P/E blocks when valid data counts are similar.
        // Normalize Erase_Count relative to the maximum expected endurance.
        IF Erase_Count >= EraseCount_Max:
            WLF = 1.0  // Avoid division by zero/overflow if maximum is reached
        ELSE:
            WLF = Erase_Count / EraseCount_Max
        // 3. Calculate Cost-Benefit Function C(B)
        // The core cost: N_Valid (minimize copy effort/WA)
        // The benefit/penalty term (WL_Penalty): penalizes blocks with low P/E count
        // C(B) = N_Valid / (1 - WLF) + W_WL * Erase_Count
        IF (1 - WLF) == 0:
            Cost_Term1 = INFINITY // Should not happen if EraseCount_Max is defined correctly
        ELSE:
            Cost_Term1 = N_Valid / (1 - WLF)  // Prioritizes low N_Valid (low WA)
        WL_Penalty = WL_Weight * Erase_Count  // Penalizes high P/E counts
        Total_Cost = Cost_Term1 + WL_Penalty
        // 4. Update Minimum Cost Block
        IF Total_Cost < Min_Cost:
            Min_Cost = Total_Cost
            Best_Victim_Block = Block
    RETURN Best_Victim_Block
// --- End of Function ---
// Example Usage/Initial Parameters (To be defined in the firmware):
// EraseCount_Max = 3000 (for TLC NAND)
// WL_Weight = 0.05
// GC_Candidate_List = [Block_1, Block_2, ..., Block_N]
// Result = Select_Victim_Block(GC_Candidate_List, 3000, 0.05, 500)