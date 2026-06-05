#ifndef STUDENT_H
#define STUDENT_H

#define MAX_NAME_LEN 50
#define MAX_USN_LEN 20
#define MAX_YEAR_LEN 20

typedef struct {
    char usn[MAX_USN_LEN];
    char name[MAX_NAME_LEN];
    char year[MAX_YEAR_LEN];
    
    int score_c_programming;
    int score_idea_lab;
    
    char skill_lab_project[MAX_NAME_LEN];
    
    int score_leadership;
    int score_teamwork;
    int score_communication;
    
    char domain[MAX_NAME_LEN];
} Student;

int load_kaggle_dataset(Student* students, int* count, const char* filename);
int add_and_save_student(Student* students, int* count, Student new_student, const char* filename);
int delete_student_from_csv(const char* identifier, const char* filename);

#endif
