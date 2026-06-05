#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "student.h"
#include "team.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 4096

Student students[1000];
int student_count = 0;

void serve_file(SOCKET client_socket, const char* path, const char* content_type) {
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "../public%s", strcmp(path, "/") == 0 ? "/index.html" : path);

    FILE* file = fopen(full_path, "rb");
    if (!file) {
        const char* response = "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found";
        send(client_socket, response, strlen(response), 0);
        return;
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* file_buf = malloc(fsize);
    fread(file_buf, 1, fsize, file);
    fclose(file);

    char header[256];
    snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n", content_type, fsize);
    
    send(client_socket, header, strlen(header), 0);
    send(client_socket, file_buf, fsize, 0);
    free(file_buf);
}

void handle_api_students(SOCKET client_socket) {
    char response_header[256];
    snprintf(response_header, sizeof(response_header), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n");
    send(client_socket, response_header, strlen(response_header), 0);

    send(client_socket, "[\n", 2, 0);
    for (int i = 0; i < student_count; i++) {
        char buf[1024];
        snprintf(buf, sizeof(buf), "  {\"usn\": \"%s\", \"name\": \"%s\", \"year\": \"%s\", \"domain\": \"%s\", \"skill_lab\": \"%s\", \"c_prog\": %d, \"idea\": %d, \"lead\": %d, \"team\": %d, \"comm\": %d}", 
            students[i].usn, students[i].name, students[i].year, students[i].domain, students[i].skill_lab_project,
            students[i].score_c_programming, students[i].score_idea_lab, students[i].score_leadership,
            students[i].score_teamwork, students[i].score_communication);
        send(client_socket, buf, strlen(buf), 0);
        
        if (i < student_count - 1) send(client_socket, ",\n", 2, 0);
        else send(client_socket, "\n", 1, 0);
    }
    send(client_socket, "]\n", 2, 0);
}

void handle_client(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        closesocket(client_socket);
        return;
    }
    buffer[bytes_received] = '\0';

    char method[16], path[256];
    sscanf(buffer, "%15s %255s", method, path);

    if (strcmp(method, "GET") == 0) {
        if (strcmp(path, "/api/students") == 0) {
            handle_api_students(client_socket);
        } else if (strstr(path, ".css") != NULL) {
            serve_file(client_socket, path, "text/css");
        } else if (strstr(path, ".js") != NULL) {
            serve_file(client_socket, path, "application/javascript");
        } else if (strstr(path, ".png") != NULL) {
            serve_file(client_socket, path, "image/png");
        } else {
            serve_file(client_socket, "/", "text/html");
        }
    } else if (strcmp(method, "POST") == 0) {
        if (strcmp(path, "/api/students") == 0) {
            char* body = strstr(buffer, "\r\n\r\n");
            if (body) body += 4;
            else body = "";

            Student s;
            memset(&s, 0, sizeof(Student));
            
            // Helper macro for naive extraction
            #define EXTRACT_STR(key, dest) \
                do { \
                    char* ptr = strstr(body, "\"" key "\":\""); \
                    if (ptr) { \
                        ptr += strlen(key) + 4; \
                        char* end = strchr(ptr, '"'); \
                        if (end) { \
                            strncpy(dest, ptr, end - ptr); \
                            dest[end - ptr] = '\0'; \
                        } \
                    } \
                } while(0)
                
            #define EXTRACT_INT(key, dest) \
                do { \
                    char* ptr = strstr(body, "\"" key "\":"); \
                    if (ptr) { \
                        ptr += strlen(key) + 3; \
                        dest = atoi(ptr); \
                    } \
                } while(0)

            EXTRACT_STR("usn", s.usn);
            EXTRACT_STR("name", s.name);
            EXTRACT_STR("year", s.year);
            EXTRACT_INT("c_prog", s.score_c_programming);
            EXTRACT_INT("idea", s.score_idea_lab);
            EXTRACT_STR("skill_lab", s.skill_lab_project);
            EXTRACT_INT("lead", s.score_leadership);
            EXTRACT_INT("team", s.score_teamwork);
            EXTRACT_INT("comm", s.score_communication);
            EXTRACT_STR("domain", s.domain);

            if (strlen(s.usn) > 0) {
                add_and_save_student(students, &student_count, s, "dataset.csv");
            }

            char response_header[256];
            snprintf(response_header, sizeof(response_header), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"status\":\"success\"}");
            send(client_socket, response_header, strlen(response_header), 0);
        } else if (strcmp(path, "/api/teams") == 0) {
            char* body = strstr(buffer, "\r\n\r\n");
            if (body) body += 4;
            else body = "";

            char domain[MAX_NAME_LEN] = "AI";
            char type[32] = "skill"; // default to skill
            
            char* domain_ptr = strstr(body, "\"domain\":\"");
            if (domain_ptr) {
                domain_ptr += 10;
                char* end = strchr(domain_ptr, '"');
                if (end) {
                    strncpy(domain, domain_ptr, end - domain_ptr);
                    domain[end - domain_ptr] = '\0';
                }
            }

            char* type_ptr = strstr(body, "\"type\":\"");
            if (type_ptr) {
                type_ptr += 8;
                char* end = strchr(type_ptr, '"');
                if (end) {
                    strncpy(type, type_ptr, end - type_ptr);
                    type[end - type_ptr] = '\0';
                }
            }

            Team teams[MAX_TEAMS];
            int team_count = 0;
            
            if (strcmp(type, "idea") == 0) {
                generate_idea_lab_teams(students, student_count, teams, &team_count);
            } else {
                generate_skill_lab_teams(students, student_count, teams, &team_count, domain);
            }

            char response_header[256];
            snprintf(response_header, sizeof(response_header), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n");
            send(client_socket, response_header, strlen(response_header), 0);

            send(client_socket, "[\n", 2, 0);
            for (int i = 0; i < team_count; i++) {
                char buf[2048];
                snprintf(buf, sizeof(buf), "  {\"id\": %d, \"domain\": \"%s\", \"compatibility\": %.2f, \"avg_c\": %.1f, \"avg_idea\": %.1f, \"avg_lead\": %.1f, \"avg_team\": %.1f, \"avg_comm\": %.1f, \"members\": [", 
                    teams[i].id, teams[i].target_domain, teams[i].compatibility_score,
                    teams[i].avg_c_programming, teams[i].avg_idea_lab, teams[i].avg_leadership, teams[i].avg_teamwork, teams[i].avg_communication);
                send(client_socket, buf, strlen(buf), 0);

                for (int j = 0; j < teams[i].num_members; j++) {
                    int idx = teams[i].student_indices[j];
                    snprintf(buf, sizeof(buf), "{\"usn\": \"%s\", \"name\": \"%s\", \"year\": \"%s\", \"score\": %d}%s", 
                        students[idx].usn, students[idx].name, students[idx].year, 
                        (students[idx].score_c_programming + students[idx].score_idea_lab + students[idx].score_leadership + students[idx].score_teamwork + students[idx].score_communication),
                        j == teams[i].num_members - 1 ? "" : ", ");
                    send(client_socket, buf, strlen(buf), 0);
                }
                send(client_socket, "]}", 2, 0);
                
                if (i < team_count - 1) send(client_socket, ",\n", 2, 0);
                else send(client_socket, "\n", 1, 0);
            }
            send(client_socket, "]\n", 2, 0);
        } else if (strcmp(path, "/api/delete_student") == 0) {
            char* body = strstr(buffer, "\r\n\r\n");
            if (body) body += 4;
            else body = "";

            char identifier[128] = "";
            char* id_ptr = strstr(body, "\"identifier\":\"");
            if (id_ptr) {
                id_ptr += 14;
                char* end = strchr(id_ptr, '"');
                if (end) {
                    strncpy(identifier, id_ptr, end - id_ptr);
                    identifier[end - id_ptr] = '\0';
                }
            }
            
            int deleted = 0;
            if (strlen(identifier) > 0) {
                deleted = delete_student_from_csv(identifier, "dataset.csv");
                if (deleted > 0) {
                    student_count = 0;
                    load_kaggle_dataset(students, &student_count, "dataset.csv");
                }
            }

            char response_header[256];
            snprintf(response_header, sizeof(response_header), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{\"status\":\"success\", \"deleted\": %d}", deleted);
            send(client_socket, response_header, strlen(response_header), 0);
        }
    }

    closesocket(client_socket);
}

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock.\n");
        return 1;
    }

    int added = load_kaggle_dataset(students, &student_count, "dataset.csv");
    if (added == 0) {
        printf("Failed to load dataset.csv! Check format.\n");
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed.\n");
        return 1;
    }

    listen(server_socket, 3);
    printf("Pure C Web Server listening on http://localhost:%d\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        int client_size = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_size);
        if (client_socket != INVALID_SOCKET) {
            handle_client(client_socket);
        }
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
