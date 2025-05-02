//
// TODO: 
//      Multi-byte chars (eventually)
//      Possibly some way to strikethough?? (not essential)
//      Sort out margin so that it is consistent
// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define ARR_LEN(a) (sizeof(a) / sizeof (a[0]))

#define MAX_PATH     4096
#define MAX_STR_LEN  62 //to account for '\n\0'
#define MAX_LINE_LEN 60 //max number of chars per line
#define MAX_LINES    30
#define MAX_PAGES    100

#define END_MSG      " ======== End of page. Press 'N' to load new page. ======== "
#define EOP_SYM      "                      ^--------------^                      "

#define ESC         27 
#define ENTER       13 
#define BACKSPACE   127 
#define NEW_PAGE    'N' 

#define	CLEAR "\033[2J\033[H"

enum {
	READ,
	WRITE,
	NEW
};

typedef struct run_vars {
    int csr_x;
    int csr_y;
    int margin_w;
    int line_count;
    int page_number;
    char curr_dir[MAX_STR_LEN];
    char file_path[MAX_PATH];
    char lines[MAX_LINES + 1][MAX_STR_LEN]; //MAX_LINES + 1 to include end-of-page message
    struct termios term_info;
    struct winsize term_winsz;
} run_vars;

struct run_vars rv;

//-------------------------------------------------//

void die(const char *s) {
  perror(s);
  exit(1);
}

void data(int rw) {
    FILE *file;
    char filename[MAX_PATH];

    if (getcwd(filename, sizeof(filename)) == NULL) {
        die("getcwd() failed");
    }

    strcpy(rv.curr_dir, strrchr(filename, '/') + 1);
    strcat(filename, "/page_");

    switch (rw) {
        case READ:
            // NOTE: find most recent file
            sprintf(rv.file_path, "%s%d", filename, rv.page_number);
            while (!(file = fopen(rv.file_path, "r+")) && rv.page_number > 1) {
                rv.page_number--;
                sprintf(rv.file_path, "%s%d", filename, rv.page_number);
            }

            if (file == NULL)
                file = fopen(rv.file_path, "w+");

            // NOTE: write contents of file to data array
            for (int i = 0; i < MAX_LINES; i++) 
                if (fgets(rv.lines[i], MAX_STR_LEN, file))
                    rv.line_count = i;
                else
                    rv.lines[i][0] = '\0';

            strcpy(rv.lines[MAX_LINES], END_MSG);
            break;

        case WRITE:
            file = fopen(rv.file_path, "w+");
            for (int i = 0; i < MAX_LINES; i++) 
                if (rv.lines[i][0]) fputs(rv.lines[i], file);
            break;

        case NEW:
            for (int i = 0; i < MAX_LINES; i++) {
                rv.lines[i][0] = '\0';
            }

            rv.line_count = 0;
            rv.page_number++;
            sprintf(rv.file_path, "%s%d", filename, rv.page_number);
            file = fopen(rv.file_path, "w+");
            break;
    }

    fclose(file);
}

void print_lines() {
    int i;
    int print_ln = 0;
    int first_line = rv.line_count - rv.csr_y;
    
    // NOTE: ideally this all needs to be printed to a buffer first 
    //       then to the screen all at once

    // NOTE: print dir name and page number at top of page
    printf("\033[%d;%dH[%s %s%d]", 
           rv.csr_y - rv.line_count - 1,
           rv.margin_w,
           rv.curr_dir,
           "Page ", 
           rv.page_number); 

    // NOTE: print lines
	for (i = 0; i <= rv.csr_y; i++) {

        print_ln = i + first_line;
        printf("\033[%d;%dH%s", 
                i + 1, // terminal row
                rv.margin_w, //move cursor to margin
                (print_ln < 0) ? "" : rv.lines[print_ln]); //print line
	}

    // NOTE: add cursor line delimeters
    printf("\033[%d;%dH>", i, rv.margin_w - 1); 
    printf("\033[%d;%dH<", i, rv.margin_w + MAX_LINE_LEN); 
    
    // NOTE: strip '\n' from cursor line - maybe this isn't the place to do this
    rv.lines[print_ln][strcspn(rv.lines[print_ln], "\n")] = '\0';

    // NOTE: print End-of-Page as it appears - this is a bit messy!
    if (MAX_LINES - rv.line_count < rv.term_winsz.ws_row - rv.csr_y) {
        if (MAX_LINES - rv.line_count) {
            printf("\033[%d;%dH%s", 
                   rv.csr_y + MAX_LINES - rv.line_count + 1,
                   rv.margin_w,
                   EOP_SYM); 
        }
    }
    
    // NOTE: put cursor at end of cursor line - always last
    rv.csr_x = (int)strlen(rv.lines[print_ln]) + rv.margin_w;
    printf("\033[%d;%dH", i, rv.csr_x); 
}

void insert_chars(char *chs) {
    int at = rv.csr_x - rv.margin_w;
    char *csr_line = rv.lines[rv.line_count];
    
    if (strlen(csr_line) + strlen(chs) < MAX_LINE_LEN) {
        strcpy(csr_line + at, chs);
        csr_line[at + strlen(chs)] = '\0';
        rv.csr_x += strlen(chs);
    }
}

void insert_char(int ch) {
    int at = rv.csr_x - rv.margin_w;
    char *csr_line = rv.lines[rv.line_count];
    
    if (strlen(csr_line) < MAX_LINE_LEN || ch =='\n') {
        csr_line[at] = ch;
        csr_line[at + 1] = '\0';
        rv.csr_x++;
    }
}

void carriage_return() {
    if (rv.line_count < MAX_LINES) {
        insert_char('\n');
        rv.line_count++;
    }
}

void do_ui() {
	printf(CLEAR);
    print_lines();
};

void do_input() {
    char ch = getchar();
    switch (ch) {
        case ESC:
            data(WRITE);
            printf(CLEAR);
            exit(0);
            break;
        case ENTER:
            carriage_return();
            break;
        case BACKSPACE:
        case '\b':
            break;
        case NEW_PAGE:
            if (rv.line_count == MAX_LINES) {
                data(WRITE);
                data(NEW);
            } else {
                insert_char(ch);
            }
            break;

            //Fallthrough to default
        case '\t':
            insert_chars("  ");
            break;
        default:
            insert_char(ch);
            break;
    }
};

void restore_terminal() { 
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &rv.term_info) == -1) die("tcgetattr"); 
}

void init_terminal() {
	if(tcgetattr(STDIN_FILENO, &rv.term_info) == -1) die("tcgetattr");
	if(ioctl(STDIN_FILENO, TIOCGWINSZ, &rv.term_winsz) == -1) die("ioctl");
    atexit(restore_terminal);

    struct termios raw = rv.term_info;

	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_lflag &= ~(ECHO | ICANON);
	raw.c_cflag |= (CS8);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    return;
};

void init_run_info() {
    rv.csr_x = 0;
    rv.csr_y = (rv.term_winsz.ws_row / 2);
    // TODO: check if terminal window is wide enough
    rv.margin_w = ((rv.term_winsz.ws_col - MAX_LINE_LEN) / 2);
    rv.line_count = 0;
    rv.page_number = MAX_PAGES;
    rv.file_path[0] = '\0';

    data(READ);
}

void run() {
    while(1) {
	    do_ui();
        do_input();
    }
};

int main() {
    init_terminal();
    init_run_info();
    run();

	return 0;
}
