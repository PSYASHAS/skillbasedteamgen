#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "student.h"
#include "team.h"
#include "utils.h"

void display_menu() {
    printf("\n=== Skill-Based Team Formation Engine ===\n");
    printf("1. Add Student Profile\n");
    printf("2. Display All Students\n");
    printf("3. Search Students by Skill\n");
    printf("4. Generate Teams\n");
    printf("5. Display Generated Teams\n");
    printf("6. Save Data\n");
    printf("7. Exit\n");
    printf("Select an option: ");
}

int main() {
    Student students[100];
    int student_count = 0;

    Team teams[MAX_TEAMS];
    int team_count = 0;

    // Try loading existing data
    student_count = load_students(students, "students.dat");
    if (student_count > 0) {
        printf("Loaded %d students from database.\n", student_count);
    }

    int choice;
    do {
        display_menu();
        if (scanf("%d", &choice) != 1) {
            clear_input_buffer();
            printf("Invalid input.\n");
            continue;
        }
        clear_input_buffer();

        switch (choice) {
            case 1:
                add_student(students, &student_count);
                break;
            case 2:
                display_students(students, student_count);
                break;
            case 3: {
                char skill[MAX_NAME_LEN];
                printf("Enter skill to search: ");
                fgets(skill, MAX_NAME_LEN, stdin);
                trim_newline(skill);
                search_students_by_skill(students, student_count, skill);
                break;
            }
            case 4: {
                char domain[MAX_NAME_LEN];
                printf("Enter target domain for teams (e.g., Web Dev, AI), or leave blank for any: ");
                fgets(domain, MAX_NAME_LEN, stdin);
                trim_newline(domain);

                int num_skills;
                printf("Enter number of required skills (max %d): ", MAX_SKILLS);
                scanf("%d", &num_skills);
                clear_input_buffer();

                if (num_skills > MAX_SKILLS) num_skills = MAX_SKILLS;
                
                char required_skills[MAX_SKILLS][MAX_NAME_LEN];
                for (int i = 0; i < num_skills; i++) {
                    printf("Enter required skill %d: ", i + 1);
                    fgets(required_skills[i], MAX_NAME_LEN, stdin);
                    trim_newline(required_skills[i]);
                }

                team_count = 0; // Reset previous teams
                generate_teams(students, student_count, teams, &team_count, domain, required_skills, num_skills);
                break;
            }
            case 5:
                display_teams(teams, team_count, students, student_count);
                break;
            case 6:
                save_students(students, student_count, "students.dat");
                break;
            case 7: {
                printf("Exiting. Do you want to save before exiting? (1 for Yes, 0 for No): ");
                int save;
                if (scanf("%d", &save) == 1 && save == 1) {
                    save_students(students, student_count, "students.dat");
                }
                printf("Goodbye!\n");
                break;
            }
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 7);

    return 0;
}
