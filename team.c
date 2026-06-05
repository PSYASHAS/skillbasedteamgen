#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "team.h"
#include "student.h"

typedef struct {
    int original_index;
    int total_score;
} EligibleStudent;

int cmp_score(const void* a, const void* b) {
    return ((EligibleStudent*)b)->total_score - ((EligibleStudent*)a)->total_score;
}

float calc_skill_score(int scores[], int num_members) {
    if (num_members == 0) return 0.0f;
    
    int max_score = 0;
    int max_idx = -1;
    
    // Find expert
    for (int i = 0; i < num_members; i++) {
        if (scores[i] > max_score) {
            max_score = scores[i];
            max_idx = i;
        }
    }
    
    float penalty = 0.0f;
    for (int i = 0; i < num_members; i++) {
        if (i == max_idx) continue;
        
        if (scores[i] <= 4) {
            penalty += 1.0f;
        } else if (scores[i] <= 7) {
            penalty += 0.5f;
        }
    }
    
    // Normalize penalty so 5-member teams aren't unfairly punished
    if (num_members > 1) {
        penalty = penalty / (num_members - 1) * (BASE_TEAM_SIZE - 1);
    }
    
    float final_score = (float)max_score - penalty;
    if (final_score < 1.0f) final_score = 1.0f;
    if (final_score > 10.0f) final_score = 10.0f;
    
    return final_score;
}

void calc_team_averages(const Student* students, Team* t) {
    if (t->num_members == 0) return;
    
    int c_scores[MAX_MEMBERS];
    int idea_scores[MAX_MEMBERS];
    int lead_scores[MAX_MEMBERS];
    int team_scores[MAX_MEMBERS];
    int comm_scores[MAX_MEMBERS];
    
    for (int j = 0; j < t->num_members; j++) {
        int idx = t->student_indices[j];
        c_scores[j] = students[idx].score_c_programming;
        idea_scores[j] = students[idx].score_idea_lab;
        lead_scores[j] = students[idx].score_leadership;
        team_scores[j] = students[idx].score_teamwork;
        comm_scores[j] = students[idx].score_communication;
    }
    
    t->avg_c_programming = calc_skill_score(c_scores, t->num_members);
    t->avg_idea_lab = calc_skill_score(idea_scores, t->num_members);
    t->avg_leadership = calc_skill_score(lead_scores, t->num_members);
    t->avg_teamwork = calc_skill_score(team_scores, t->num_members);
    t->avg_communication = calc_skill_score(comm_scores, t->num_members);

    float total_avg = (t->avg_c_programming + t->avg_idea_lab + 
                       t->avg_leadership + t->avg_teamwork + 
                       t->avg_communication) / 5.0f;
                       
    t->compatibility_score = (total_avg / 10.0f) * 100.0f;
}

float get_global_variance(Team* teams, int team_count) {
    float sum = 0;
    for (int i = 0; i < team_count; i++) sum += teams[i].compatibility_score;
    float mean = sum / team_count;
    float variance = 0;
    for (int i = 0; i < team_count; i++) {
        float diff = teams[i].compatibility_score - mean;
        variance += diff * diff;
    }
    return variance;
}

void optimize_teams(const Student* students, Team* teams, int team_count, int is_idea_lab) {
    if (team_count <= 1) return;
    
    srand((unsigned int)time(NULL));
    int iterations = 200000; // 200k iterations for global variance
    
    float current_variance = get_global_variance(teams, team_count);
    
    for (int iter = 0; iter < iterations; iter++) {
        int t1 = rand() % team_count;
        int t2 = rand() % team_count;
        if (t1 == t2) continue;
        
        int members1 = teams[t1].num_members;
        int members2 = teams[t2].num_members;
        if (members1 == 0 || members2 == 0) continue;
        
        int k1 = rand() % members1;
        int k2 = rand() % members2;
        
        if (is_idea_lab) {
            if (k1 == 0 && k2 != 0) continue;
            if (k2 == 0 && k1 != 0) continue;
            if (k1 == 1 && k2 != 1) continue;
            if (k2 == 1 && k1 != 1) continue;
        }
        
        // Swap
        int temp = teams[t1].student_indices[k1];
        teams[t1].student_indices[k1] = teams[t2].student_indices[k2];
        teams[t2].student_indices[k2] = temp;
        
        calc_team_averages(students, &teams[t1]);
        calc_team_averages(students, &teams[t2]);
        
        float new_variance = get_global_variance(teams, team_count);
        
        if (new_variance < current_variance) {
            // Keep swap
            current_variance = new_variance;
        } else {
            // Revert swap
            temp = teams[t1].student_indices[k1];
            teams[t1].student_indices[k1] = teams[t2].student_indices[k2];
            teams[t2].student_indices[k2] = temp;
            
            calc_team_averages(students, &teams[t1]);
            calc_team_averages(students, &teams[t2]);
        }
    }
}

void generate_idea_lab_teams(const Student* students, int count, Team* teams, int* team_count) {
    *team_count = count / BASE_TEAM_SIZE;
    if (*team_count > MAX_TEAMS) *team_count = MAX_TEAMS;
    if (*team_count == 0) return;

    for (int i = 0; i < *team_count; i++) {
        teams[i].id = i + 1;
        strncpy(teams[i].target_domain, "Cross-Domain", MAX_NAME_LEN);
        teams[i].num_members = 0;
    }

    int used[1000] = {0};

    // 1. Assign 1 Strong Leader (>= 8) to each team
    for (int i = 0; i < *team_count; i++) {
        for (int j = 0; j < count; j++) {
            if (!used[j] && students[j].score_leadership >= 8) {
                teams[i].student_indices[teams[i].num_members++] = j;
                used[j] = 1;
                break;
            }
        }
    }

    // 2. Assign 1 Strong Communicator (>= 8) to each team
    for (int i = 0; i < *team_count; i++) {
        for (int j = 0; j < count; j++) {
            if (!used[j] && students[j].score_communication >= 8) {
                teams[i].student_indices[teams[i].num_members++] = j;
                used[j] = 1;
                break;
            }
        }
    }

    // 3. Pool remaining unused students and sort by overall performance
    EligibleStudent eligible[1000];
    int num_eligible = 0;
    for (int j = 0; j < count; j++) {
        if (!used[j]) {
            eligible[num_eligible].original_index = j;
            // Prioritize overall performance
            eligible[num_eligible].total_score = students[j].score_c_programming + 
                                                 students[j].score_idea_lab + 
                                                 students[j].score_leadership + 
                                                 students[j].score_teamwork + 
                                                 students[j].score_communication;
            num_eligible++;
        }
    }

    qsort(eligible, num_eligible, sizeof(EligibleStudent), cmp_score);

    // 4. Distribute evenly using a Two-Phase Snake Draft
    // Phase 1: Force every team to hit exactly BASE_TEAM_SIZE (4) first
    int dir = 1;
    int t_idx = 0;
    int i = 0;
    
    while (i < num_eligible) {
        int all_filled = 1;
        for (int k = 0; k < *team_count; k++) {
            if (teams[k].num_members < BASE_TEAM_SIZE) {
                all_filled = 0;
                break;
            }
        }
        if (all_filled) break; // Move to phase 2
        
        if (teams[t_idx].num_members < BASE_TEAM_SIZE) {
            teams[t_idx].student_indices[teams[t_idx].num_members++] = eligible[i].original_index;
            i++;
        }
        
        t_idx += dir;
        if (t_idx >= *team_count) {
            t_idx = *team_count - 1;
            dir = -1;
        } else if (t_idx < 0) {
            t_idx = 0;
            dir = 1;
        }
    }
    
    // Phase 2: Distribute leftovers as 5th/6th members
    dir = 1;
    t_idx = 0;
    while (i < num_eligible) {
        if (teams[t_idx].num_members < MAX_MEMBERS) {
            teams[t_idx].student_indices[teams[t_idx].num_members++] = eligible[i].original_index;
            i++;
        } else {
            // Prevent infinite loop if all teams somehow hit MAX_MEMBERS
            int all_maxed = 1;
            for (int k = 0; k < *team_count; k++) {
                if (teams[k].num_members < MAX_MEMBERS) all_maxed = 0;
            }
            if (all_maxed) break;
        }
        
        t_idx += dir;
        if (t_idx >= *team_count) {
            t_idx = *team_count - 1;
            dir = -1;
        } else if (t_idx < 0) {
            t_idx = 0;
            dir = 1;
        }
    }

    // Calculate scores
    for (int i = 0; i < *team_count; i++) {
        calc_team_averages(students, &teams[i]);
    }

    optimize_teams(students, teams, *team_count, 1);
}

void generate_skill_lab_teams(const Student* students, int count, Team* teams, int* team_count, const char* target_domain) {
    EligibleStudent eligible[1000];
    int num_eligible = 0;

    for (int i = 0; i < count; i++) {
        if (strstr(students[i].domain, target_domain) != NULL) {
            eligible[num_eligible].original_index = i;
            eligible[num_eligible].total_score = students[i].score_c_programming + 
                                                 students[i].score_leadership + 
                                                 students[i].score_teamwork + 
                                                 students[i].score_communication;
            num_eligible++;
        }
    }

    if (num_eligible < BASE_TEAM_SIZE) {
        *team_count = 0;
        return;
    }

    qsort(eligible, num_eligible, sizeof(EligibleStudent), cmp_score);

    *team_count = num_eligible / BASE_TEAM_SIZE;
    if (*team_count > MAX_TEAMS) *team_count = MAX_TEAMS;

    for (int i = 0; i < *team_count; i++) {
        teams[i].id = i + 1;
        strncpy(teams[i].target_domain, target_domain, MAX_NAME_LEN);
        teams[i].num_members = 0;
    }

    // Snake Draft (Two-Phase to guarantee 4-members first)
    int dir = 1;
    int t_idx = 0;
    int i = 0;
    
    while (i < num_eligible) {
        int all_filled = 1;
        for (int k = 0; k < *team_count; k++) {
            if (teams[k].num_members < BASE_TEAM_SIZE) {
                all_filled = 0;
                break;
            }
        }
        if (all_filled) break; 
        
        if (teams[t_idx].num_members < BASE_TEAM_SIZE) {
            teams[t_idx].student_indices[teams[t_idx].num_members++] = eligible[i].original_index;
            i++;
        }
        
        t_idx += dir;
        if (t_idx >= *team_count) {
            t_idx = *team_count - 1;
            dir = -1;
        } else if (t_idx < 0) {
            t_idx = 0;
            dir = 1;
        }
    }
    
    // Phase 2: Leftovers
    dir = 1;
    t_idx = 0;
    while (i < num_eligible) {
        if (teams[t_idx].num_members < MAX_MEMBERS) {
            teams[t_idx].student_indices[teams[t_idx].num_members++] = eligible[i].original_index;
            i++;
        } else {
            int all_maxed = 1;
            for (int k = 0; k < *team_count; k++) {
                if (teams[k].num_members < MAX_MEMBERS) all_maxed = 0;
            }
            if (all_maxed) break;
        }
        
        t_idx += dir;
        if (t_idx >= *team_count) {
            t_idx = *team_count - 1;
            dir = -1;
        } else if (t_idx < 0) {
            t_idx = 0;
            dir = 1;
        }
    }

    for (int i = 0; i < *team_count; i++) {
        calc_team_averages(students, &teams[i]);
    }

    optimize_teams(students, teams, *team_count, 0);
}
