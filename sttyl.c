//Isaac Fowler - 2/24/2022

/* This program demonstrates how tables and the termios structure work. Sttyl can directly change various terminal related settings using flags */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

//ac tells how many elements are in av[] (argument count)
//av[] is an array of strings that holds command line arguments (argument vector)

void noArgs(struct termios *terminal);
void Args(struct termios *terminal, char * av[], int ac);


struct flaginfo { int fl_value; char *fl_name; };

struct flaginfo input_flags[] = {
        IGNBRK  ,       "ignbrk",
        BRKINT  ,       "brkint",
        IGNPAR  ,       "ignpar",
        PARMRK  ,       "parmrk",
        INPCK   ,       "inpck",
        ISTRIP  ,       "istrip",
        INLCR   ,       "inlcr",
        ICRNL   ,       "icrnl",
        IXON    ,       "ixon",
        IXOFF   ,       "ixoff",
        0       ,       NULL
};

struct flaginfo local_flags[] = {
        ISIG    ,       "isig",
        ICANON  ,       "icanon",
        ECHO    ,       "echo",
        ECHOE   ,       "echoe",
        ECHOK   ,       "echok",
        0       ,       NULL
};

struct flaginfo output_flags[] = {
        ONLCR   ,       "onlcr",
        OLCUC   ,       "olcuc",
        TAB0    ,       "tabs",
        0       ,       NULL
};

struct flaginfo special_characters[] = {
        VINTR   ,       "intr",
        VERASE  ,       "erase",
        VKILL   ,       "kill",
        0       ,       NULL
};

int main(int ac, char* av[]) {

        struct termios terminal; //terminal structure to hold attributes

        if(tcgetattr(STDIN_FILENO, &terminal) == -1) {
                perror("Could not get current settings from terminal driver");
                exit(1);
        }

        if (ac == 1) {
                noArgs(&terminal); //Go to no aguments function
                printf("\n");
        } else if (ac > 1) {
                Args(&terminal, av, ac); //Go to arguments function
        }

        if(tcsetattr(STDIN_FILENO, TCSANOW, &terminal) == -1) { //Set the changes made!
                perror("Could not write new settings to terminal driver");
                exit(1);
        }

        //process complete
        return 0;
}

void noArgs(struct termios *terminal) {

        showbaud(cfgetospeed(terminal));
        char intr = terminal->c_cc[special_characters[0].fl_value];
        if (intr == 3) intr = intr+64; //Fix inability to print control character by changing ASCII
        char erase = terminal->c_cc[special_characters[1].fl_value];
        if (erase == 21) erase = terminal->c_cc[VERASE]-1+'A'; //Fix inability to print control character by changing ASCII
        if (erase == 127) //if it is the delete control character
                erase = '?'; //make it a question mark
        char kill = terminal->c_cc[special_characters[2].fl_value];
        if (kill == 21) kill = terminal->c_cc[VKILL]-1+'A';
        printf("intr = ^%c; erase = ^%c; kill = ^%c;\n", intr, erase, kill);
        //printf("intr = ^%c; erase = ^%c; kill = ^%c;\n", terminal->c_cc[VINTR]+64, terminal->c_cc[VERASE]+64, terminal->c_cc[VKILL]-1+'A');

        //printf("new char is: %c\n", terminal->c_cc[special_characters[0].fl_value]);
        show_some_flags(terminal);

}

void Args(struct termios *terminal, char * av[], int ac) {
        int index;
        for (int i = 1; i < ac; ++i) {

                int negate = 0;
                int unknown = 0;
                char newChar;
                if (av[i][0] == '-') {
                        negate = 1; //negate is detected
                }
                index = findTableIndex(input_flags, av[i]); //check input flags
                if (index >= 0) {
                        //printf(" (you entered an input flag) ");
                        if (negate == 0) {
                                terminal->c_iflag |= input_flags[index].fl_value;
                                printf("turned on %s\n", av[i]);
                        }
                        else if (negate == 1) {
                                terminal->c_iflag &= ~(input_flags[index].fl_value);
                                printf("turned off %s\n", input_flags[index].fl_name);
                        }

                } else if (index == -1) { //if no input flag, increment unknown
                        ++unknown;
                }
                index = findTableIndex(local_flags, av[i]); //check local flags
                if (index >= 0) {
                        //printf(" (you entered a local flag) ");
                        if (negate == 0) {
                                terminal->c_lflag |= local_flags[index].fl_value;
                                printf("turned on %s\n", av[i]);
                        } else if (negate == 1) {
                                terminal->c_lflag &= ~(local_flags[index].fl_value);
                                printf("turned off %s\n", local_flags[index].fl_name);
                        }
                } else if (index == -1) { //if no local flag, increment unknown
                        ++unknown;
                }
                index = findTableIndex(output_flags, av[i]); //check output flags
                if (index >= 0) {
                        if (negate == 0) {
                                terminal->c_oflag |= output_flags[index].fl_value;
                                printf("Turned on %s\n", av[i]);
                        } else if (negate == 1) {
                                terminal->c_oflag &= ~(output_flags[index].fl_value);
                                printf("Turned off %s\n", output_flags[index].fl_name);
                        }
                } else if (index == -1) { //if no output flag, increment unknown
                        ++unknown;
                }
                index = findTableIndex(special_characters, av[i]);
                if (index >= 0) {
                        if (i+1 >= ac) { //if the next iteration of the loop is greater than or equal to the amount of arguments
                                fprintf(stderr, "Missing a character after %s\n", av[i]); //then its missing a character for the control flag
                                exit(1);
                        }

                        newChar = av[i+1][0];
                        if (newChar == '^') //this enables compatability with ^ input
                                newChar = av[i+1][1]; //set to next character index of argument string
                        terminal->c_cc[special_characters[index].fl_value] = newChar;
                        printf("Set %s to ^%c\n", special_characters[index].fl_name, newChar);
                        ++i;
                } else if (index == -1) {
                        ++unknown;
                }


                if (unknown == 4) { //Unknown is incremented each time it isn't a flag. If it is incremented 4 times then the argument hasn't been found.
                        printf("unknown mode: '%s'\n", av[i]);
                }
                //printf(" av is %s\n", av[i]);
        }
}

int findTableIndex(struct flaginfo flagnames[], char *name) {
        char * fixedName = malloc(sizeof(name)-1);
        char * test;
        if (name[0] == '-') { //if it starts with a dash
                for (int i = 0; i < strlen(name)-1; ++i) {
                        fixedName[i] = name[i+1]; //fix the name to have no dash in the front by shifting to the left
                }
        } else {
                fixedName = name;
        }
        for (int i = 0; (flagnames[i].fl_name); ++i) {
                if(strcmp(flagnames[i].fl_name, fixedName) == 0) { //if the flag and fixed name match
                        //printf("Debug: Match found at: %d ", i);
                        return i; //return the index
                }
        }
        return -1;
}

showbaud(int thespeed) {
        printf("speed ");
        switch (thespeed) {
                case B300:      printf("300");          break;
                case B600:      printf("600");          break;
                case B1200:     printf("1200");         break;
                case B1800:     printf("1800");         break;
                case B2400:     printf("2400");         break;
                case B4800:     printf("4800");         break;
                case B9600:     printf("9600");         break;
                case B38400:    printf("38400");        break;
                default:        printf("Fast");         break;
        }
        printf(" baud;\n");
}

show_some_flags( struct termios *terminal) {
        show_flagset(terminal->c_iflag, input_flags); //input flags table
        show_flagset(terminal->c_lflag, local_flags); //local flags table
}

show_flagset(int thevalue, struct flaginfo thebitnames[]) {
        int i;
        int count = 0;
        for (i = 0; thebitnames[i].fl_value; ++i) {
                if (thevalue & thebitnames[i].fl_value)
                        printf("%s ", thebitnames[i].fl_name);
                else
                        printf("-%s ", thebitnames[i].fl_name);
                ++count;
                if (count == 6) {
                        printf("\n"); //print 6 flags per line
                        count = 0;
                }

        }
}
