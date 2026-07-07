all: grades

grades : grades.c
        gcc grades.c -o grades

clean:
        rm grades
