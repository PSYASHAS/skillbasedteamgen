#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "student.h"
#include "utils.h"

int load_kaggle_dataset(Student* students, int* count, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Could not open Kaggle dataset: %s\n", filename);
        return 0;
    }

    char line[2048];
    int added = 0;
    *count = 0; 
    
    // Skip header
    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        return 0;
    }

    while (fgets(line, sizeof(line), file)) {
        trim_newline(line);
        if (strlen(line) == 0) continue;

        Student s;
        memset(&s, 0, sizeof(Student));

        char* token = strtok(line, ",");
        if (!token) continue;
        strncpy(s.usn, token, MAX_USN_LEN);

        token = strtok(NULL, ",");
        if (!token) continue;
        strncpy(s.name, token, MAX_NAME_LEN);

        token = strtok(NULL, ",");
        if (!token) continue;
        strncpy(s.year, token, MAX_YEAR_LEN);

        token = strtok(NULL, ",");
        if (!token) continue;
        s.score_c_programming = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        s.score_idea_lab = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        strncpy(s.skill_lab_project, token, MAX_NAME_LEN);

        token = strtok(NULL, ",");
        if (!token) continue;
        s.score_leadership = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        s.score_teamwork = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        s.score_communication = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        strncpy(s.domain, token, MAX_NAME_LEN);

        students[*count] = s;
        (*count)++;
        added++;
    }
    fclose(file);
    printf("Successfully loaded %d students from Kaggle dataset.\n", added);
    return added;
}

int add_and_save_student(Student* students, int* count, Student new_student, const char* filename) {
    if (*count >= 1000) return 0; // Max students
    
    students[*count] = new_student;
    (*count)++;

    FILE* file = fopen(filename, "a");
    if (!file) return 0;

    // USN,Student_Name,Year,C_Prog,Idea_Lab,Skill_Lab,Leadership,Teamwork,Comm,Domain
    fprintf(file, "%s,%s,%s,%d,%d,%s,%d,%d,%d,%s\n", 
        new_student.usn, new_student.name, new_student.year, 
        new_student.score_c_programming, new_student.score_idea_lab, 
        new_student.skill_lab_project, new_student.score_leadership, 
        new_student.score_teamwork, new_student.score_communication, 
        new_student.domain);
    
    fclose(file);
    return 1;
}

int delete_student_from_csv(const char* identifier, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return 0;
    
    FILE* temp = fopen("temp.csv", "w");
    if (!temp) {
        fclose(file);
        return 0;
    }
    
    char line[2048];
    int deleted = 0;
    
    if (fgets(line, sizeof(line), file)) {
        fputs(line, temp);
    }
    
    while (fgets(line, sizeof(line), file)) {
        char line_copy[2048];
        strcpy(line_copy, line);
        
        trim_newline(line_copy);
        char* usn = strtok(line_copy, ",");
        char* name = strtok(NULL, ",");
        
        if (usn && name && (strcmp(usn, identifier) == 0 || strcmp(name, identifier) == 0)) {
            deleted++;
        } else {
            fputs(line, temp);
        }
    }
    
    fclose(file);
    fclose(temp);
    
    if (deleted > 0) {
        remove(filename);
        rename("temp.csv", filename);
    } else {
        remove("temp.csv");
    }
    
    return deleted;
}
