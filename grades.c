#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct Node Node;

struct GradeEntry {
  char studentId[11];
  char assignmentName[21];
  unsigned short grade;
};

struct Node {
  struct GradeEntry gradeEntry;
  struct Node *next;
};

int main (int argc, char *argv[]) {
  if (argc != 2) {
    printf("Please provide one file to a database.\n");
    exit(1);
  }

  Node *first = NULL;
  Node *last = NULL;

  FILE *file;

  file = fopen(argv[1], "r");
  if (file == NULL) {
    printf("File does not exist.\n");
    exit(1);
  }

  char line[256];

  while (fgets(line, sizeof(line), file)) {
    struct Node *new_node;
    new_node = malloc(sizeof(struct Node));

    if (new_node == NULL) {
      printf("Allocation failed.");
      exit(1);
    }

    new_node->next = NULL;

    if (first == NULL) {
      first = new_node;
      last = new_node;
    }
    else {
      last->next = new_node;
      last = new_node;
    }

    char *token;

    token = strtok(line, ":");
    strcpy(new_node->gradeEntry.studentId, token);

    token = strtok(NULL, ":");
    strcpy(new_node->gradeEntry.assignmentName, token);

    token = strtok(NULL, ":");
    int grade = atoi(token);
    new_node->gradeEntry.grade = grade;
  }

  fclose(file);

  while (1) {
    char input[50];
    char *command;
    char *argument;

    if (fgets(input, sizeof(input), stdin) == NULL) {
      char temp[] = "gradesXXXXXX";
      int fd;

      fd = mkstemp(temp);
      if (fd == -1) {
        printf("Creating temporary file failed.\n");
        exit(1);
      }

      FILE *tempFile = fdopen(fd, "w");

      if (tempFile == NULL) {
        unlink(temp);
        printf("Failed to open temporary file.\n");
        exit(1);
      }

      Node *current;

      for (current = first; current != NULL; current = current->next) {
        if (fprintf(tempFile, "%s:%s:%hu\n", current->gradeEntry.studentId, current->gradeEntry.assignmentName, current->gradeEntry.grade) < 0) {
          fclose(tempFile);
          unlink(temp);
          printf("Failed to write temporary file.\n");
          exit(1);
        }
      }

      if (rename(temp, argv[1]) != 0) {
        unlink(temp);
        printf("Failed to replace original file.\n");
        exit(1);
      }

      current = first;

      while (current != NULL) {
        Node *next = current->next;
        free(current);
        current = next;
      }

      break;
    }

    input[strcspn(input, "\n")] = '\0';
    command = strtok(input, " ");
    argument = strtok(NULL, "\0");

    if (strncmp(command, "add", 3) == 0) {
      char *studentId = strtok(argument, ":");
      char *assignment = strtok(NULL, ":");
      char *grade = strtok(NULL, ":");

      if (studentId == NULL || assignment == NULL || grade == NULL) {
        printf("Invalid argument.\n");
        continue;
      }

      if (strlen(studentId) != 10) {
        printf("Invalid student ID.\n");
        continue;
      }

      int invalidId = 0;

      for (int i=0; i < 10; i++) {
        if (studentId[i] < '0' || studentId[i] > 9) {
          invalidId = 1;
          break;
        }
      }

      if (invalidId == 1) {
        printf("Student ID must be made of digits.\n");
        continue;
      }

      if (strlen(assignment) > 20) {
        printf("Assignment name is too long.\n");
        continue;
      }

      int gradeNum = atoi(grade);
      if (gradeNum < 0 || gradeNum > 100) {
        printf("Invalid grade.\n");
        continue;
      }

      Node *current;

      int duplicateFound = 0;

      for (current = first; current != NULL; current = current->next) {
        if (strcmp(current->gradeEntry.studentId, studentId) == 0 && strcmp(current->gradeEntry.assignmentName, assignment) == 0) {
          duplicateFound = 1;
          break;
        }
      }

      if (duplicateFound == 1) {
        printf("Entry for this assignment already exists.\n");
        continue;
      }

      struct Node *new_node;
      new_node = malloc(sizeof(struct Node));

      if (new_node == NULL) {
        printf("Allocation failed.");
        exit(1);
      }

      new_node->next = NULL;

      if (first == NULL) {
        first = new_node;
        last = new_node;
      }
      else {
        last->next = new_node;
        last = new_node;
      }

      strcpy(new_node->gradeEntry.studentId, studentId);
      strcpy(new_node->gradeEntry.assignmentName, assignment);
      new_node->gradeEntry.grade = gradeNum;
    }

    else if(strcmp(command, "remove") == 0) {
      char *studentId = strtok(argument, ":");
      char *assignment = strtok(NULL, "");

      if (studentId == NULL || assignment == NULL) {
        printf("Invalid argument.\n");
        continue;
      }

      if (strlen(studentId) != 10) {
        printf("Invalid student ID.\n");
        continue;
      }

      if (strlen(assignment) > 20) {
        printf("Assignment name is too long.\n");
        continue;
      }

      Node *current;
      Node *previous;

      int entryFound = 0;

      for (current = first; current != NULL; current = current->next) {
        if (strcmp(current->gradeEntry.studentId, studentId) == 0 && strcmp(current->gradeEntry.assignmentName, assignment) == 0) {
          if (previous == NULL) {
            first = current->next;
          }
          else {
            previous->next = current->next;
          }

          free(current);
          entryFound = 1;
          break;
        }

        previous = current;
      }

      if (entryFound == 0) {
        printf("Assignment not found.\n");
        continue;
      }
    }

    else if (strcmp(command, "print") == 0) {
      printf("Student ID | Assignment Name      | Grade\n");
      printf("-----------------------------------------\n");

      Node *current = first;

      while (current != NULL) {
        printf("%-10s | %-20s | %hu\n", current->gradeEntry.studentId, current->gradeEntry.assignmentName, current->gradeEntry.grade);
        current = current->next;
      }
    }

    else if (strcmp(command, "stats") == 0) {
      if (argument == NULL) {
        printf("Please provide an assignment.\n");
        continue;
      }

      Node *current;

      int max = 0;
      int min = 100;
      double mean = 0;
      int count = 0;

      for (current = first; current != NULL; current = current->next) {
        if (strcmp(current->gradeEntry.assignmentName, argument) == 0) {
          if (current->gradeEntry.grade > max) {
            max = current->gradeEntry.grade;
          }
          if (current->gradeEntry.grade < min) {
            min = current->gradeEntry.grade;
          }

          mean += current->gradeEntry.grade;
          count++;
        }
      }

      if (count == 0) {
        printf("No grade entries for %s\n", argument);
        continue;
      }

      mean = mean / count;
      printf("Grade statistics for %s\n", argument);
      printf("Min: %d\n", min);
      printf("Max: %d\n", max);
      printf("Mean: %.2f\n", mean);
    }

    else {
      printf("Please provide a valid command.\n");
    }
  }

  return 0;
}
