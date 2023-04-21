
# ECS 150 Spring 2023
## Project 1: Simple Shell
### Daksh Mathur, Manya Murali

## Summary
Our project spanned multiple days following the outlined phases that were
provided in the project document. For this project, we were tasked with creating
a simple shell from scratch. 

The simple shell acts as a command line interpreter and creates an interface
between the operating system and the user. We were also able to understand the
shell is at the core of the operating system. This project allowed for
understanding between different UNIX system calls and execute them as jobs.
Additionally, in completing this project, we were able to understand how
different processes interact in the background.

## Implementation
Although it was recommended to use structs,from the beginning our implementation
did not include structs but instead we made many functions. However, we still
have a strong understanding of how our functions interact with each other and
were able to make an overall organized main implementation in our sshell.c file.

<h5 a><strong><code>
"#include "helper.h"
int main(void)
{
        start(); // starting the shell
}</code></strong></h5>

## Parsing Arguments and Executing Commands
In this phase of the project, we created a function process() that removes
leading and trailing whitespaces the given function parameter. We also used
strtok_r() to separate commands from their arguments. We call strtok_r() and
process() function many times in our program to make sure everytime we receive a
command line it is in the proper format for execution. Something we should have
done from the beginning is that we should have made a parsing function that
would be called only once from all functions. In our implementation we are
calling strtok_r() and process() multiple times from a single function, this
could have been avoided if we had a parsing function. We also made the decision
to use strtok_r() instead of strtok() because we wanted to split up our command
line so we can store both the command and the arguments in character pointers.
With strtok_r() we were able to keep the arguments in the character pointer
passed to strtok_r().

In order to properly execute with different arguments and to decipher and parse
the commands itself, we created the function command(). In this particular
function, we utilize both execvp() and execlp() to take in the command line as
both a string array and a string list. When we tested our code with tester.sh we
realized that the command mkdir -p was causing a problem and not executing
properly. This made us realize that we need to be able to execute a command with
a list of arguments. This is why we added the execlp() function.It was a simple
addition that could have been missed had we not tested our code. 

## Functions

We started our helper.h file with the start() function. This function was used
to put all the code from main so that we could reduce clutter in main. This is
where we put all the skeleton code at the beginning. All the functions are
called from this function. It can be considered a main imitator function. 

<h5 a><strong><code>void start(){}</code></strong></h5>

The next objective was to replace system() with the fork()+exec()+wait() method.
We made the command() function that took the command line and executed it
correctly. In order to incorporate for extra whitespaces we created the
process() function and used it along with strtok_r() inside command(). After
forking we used child to execute and parent to print out the completed message
along with the exit status.

<h5 a><strong><code>void command(char *cmd){}</code></strong></h5>
<h5 a><strong><code>void process(char *cmd){}</code></strong></h5>

After that step, we created the builtin() function. The main objective of the
builtin() function was to implement "pwd", "exit", and "cd"(later "save")
commands as a core feature. To use "exit" we returned a 1 from builtin() and
that lead to a break command that broke out of the while loop. For "pwd", we
used the getcwd() function to get the correct working directory and then we
printed it out to stdout stream to inform the user. Lastly for "cd" we used the
chdir() function and we parsed the command line to switch to the correct
directory. If the directory doesn't exist we showed a user friendly error.
<h5 a><strong><code>int builtin(char* cmd){}(char* cmd){}</code></strong></h5>

After getting a better idea of syscalls from the builtin commands and normal
commands, we moved on to create our redirect() function. We parsed command line
for '>' character. If it appeared we determined that we have to redirect,
otherwise shell continues the same. Later on in our implementation, we added the
extra feature of combined output redirection . We used strstr() to have the
pointer return the first occurence of ">". Afterwards, we used an int fd to
represent the file stream and redirected the stdout stream to the file using
dup2().In the extra feature we also redirected the stderr stream to a file using
the same method. We also used the dup() function to revert stdout and stderr
back to their respective streams. This allowed for the shell to function
properly.

<h5 a><strong><code>int redirect(char* cmd){}(char* cmd){}</code></strong></h5>

After understanding how different streams can be redirected and reverted we work
on piping. This was the hardest part of the program for us. We tried to
implement it with a for loop as we did not realize that there was a limit to how
many pipes there could be. We were successful in creating a pipe for two
commands but our time management was not the best and we had to use if else
statements to create up to three pipes. The function we created for this is
called pipeline() and we created another function similar to command() to do
pipelined commands called pipCommand(). The main difference was executing in
parallel and printing out all the exit status from the children processes and
the actual output from the piped commands. This gave us a very good
understanding of how Linux/Unix actually does piping behind the scenes. 

<h5 a><strong><code>int pipeline(char *cmd){}</code></strong></h5>
<h5 a><strong><code>void pipCommand(char *cmd,int last)</code></strong></h5>

Finally we worked on the last extra feature where we had to create environment
variables on the command line. As we said earlier our implementation does not
have structs so this extra feature was a bit tricky to implement. We came up
with the idea to make a character pointer array to store the values of the
variables. We were able to set the value but due to it being a pointer the point
in memory where it was pointing changed as we wrote other commands and the shell
proceeded. With an implementation with structs we would be able to implement
this feature as fully functional and would also be able to create a pipeline
function that is more efficient and flexible for multiple pipes without limits.

## Error Handling
The function that handles parsing errors is called parseError(). 

We started working on error handling after we finished our pipeline function and
before we worked on environment variables. We worked on error handling after
finishing most of our functions because it gave us a better understanding of how
each of our functions worked and what exactly makes them fail. Knowing the exact
requirements and special cases gave us enough understanding to cover most if not
all possible errors in our shell. We weren't able to implement the error
handling for a file that we don't have access to. This happened because when we
implemented this error handling the ability to create a file for output
redirection was taken away and it was causing incorrect redirection. With better
time management we could implement this error handling and not cause incorrect
redirection.

An example of how we handled an error is the too many arguments case. We used
process() and strtok() to count the number of arguments on the command line. If
the number exceeded 16 elements, we print out the error message "Error: too many
process arguments" to stderr stream. We used similar approaches to successfully
handle all the other parsing errors except the "unable to open file" error.
parseError() was not the only function that did error handling. In our command()
function we used our understanding that execvp and execlp only return to the
same spot if they fail. We printed to stderr stream to indicate to the user that
the command they input was not found. Our builtin() function also dealt with
errors, as mentioned before, if the directory entered was not found then we
inform the user that we cannot change into the directory. 

## Pipelining
Here we just wanted to clarify our piping implementation a little with some
details. As we did not manage our time properly with had to give up on the for
loop implementation and we kept the if else if implementation. We had to use the
same lines of code up to three times due to this approach and this made our
program file much longer. In the implementation we used a variable track that
parsed the command for a pipe, "|" and increased our counter variable whenever a
"|" was found. Then, we created four instances of pid_t: pid1, pid2, pid3, pid4
and we created three instance of pipes: pipe(fd1),pipe(fd2) and pipe(fd3). For
the first pipe, when count == 1, we used pid1 to fork() and redirect the
respective file desriptors. The same process was repeated for when pid2 was
equal to 0 and we forked. This process was repeated multiple times until pid4
was reached and count was equal to 3. The repetitions depend on how many '|'
appear in the command line. Each '|' corresponds to a pipe generation.

## Testing
In order to properly test our code, we used all the test cases in the provided
tester script (tester.sh file). We also tested our shell with the commands that
were provided on the project document and tested each phase to see if our
outputs matched the results in the project description. The program was also
tested on the CSIF computers to ensure that it worked in the CSIF environment.
During the testing phase a lot of things were cleared up for us, an example
being the implementation of execlp as mentioned before.This was the most
important phase for us because we were able to confirm our theories and remove
our doubts. We used various print statements to correct wherever our functions
were misbehaving. Without testing the code it would have been difficult to pass
any of the test cases. Unfortunately, we did not test for memory leaks using
valgrind or another testscript. We do know that without structs for this shell
our memory management is not efficient at all and we definitely are using a lot
of memory but we will be able to improve on this as we progress through our
operating systems class.

## Sources
Relying on lecture slides and other online resources greatly helped us in
achieving our goals. 

We are very grateful to Professor Porquet-Lupine for his lecture notes, videos,
and slides for aiding us in our implementation. Additionally, here are some
sources that we used for examples and understanding in utilization: 

[The GNU C
Library](https://www.gnu.org/software/libc/manual/html_mono/libc.html#Search-Functions)

[Linux Manual Page](https://man7.org/linux/man-pages/man3/system.3.html)

[Stack Overflow](https://stackoverflow.com/)