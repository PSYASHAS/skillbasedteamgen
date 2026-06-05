#ifndef TEAM_H
#define TEAM_H

#include "student.h"

#define MAX_MEMBERS 6  // Expanded capacity for overflow
#define BASE_TEAM_SIZE 4 // Core division math
#define MAX_TEAMS 100

typedef struct {
    int id;
    char target_domain[MAX_NAME_LEN];
    
    int student_indices[MAX_MEMBERS];
    int num_members;
    
    float avg_c_programming;
    float avg_idea_lab;
    float avg_leadership;
    float avg_teamwork;
    float avg_communication;
    
    float compatibility_score;
} Team;

void generate_idea_lab_teams(const Student* students, int count, Team* teams, int* team_count);
void generate_skill_lab_teams(const Student* students, int count, Team* teams, int* team_count, const char* target_domain);

#endif
