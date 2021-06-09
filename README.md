# SimpleShell |'L'|
A practice of exploring how Unix shells are made. 

### Logo`|'L'|`

The prompt for the user to enter an command will be `|'L'|$ `. The design is based on the finder app on MacOS. 

### Makefile

Makefile is included. Supporting functions might be migrated into a different `.cpp` file in the future. For version 1.0, they are kept in the same file. 

### Built-in Commands

Currently, there are the following built-in commands supported:

- `cd` - Change directory.

  - Always take one argument. 

- `path` - Overwrites old paths with newly specified paths. 

  - Takes 0 or more arguments. Each should be separated with whitespace from the others. 
  - If called without any arguments, the shell cannot execute any commands excepts the built-in ones.

- `exit` - Exit with `0`.

  

`clear` - Coming soon.... 

### Path

The default path that the shell will look for system program such as `ls` will be `/bin/`. 

You can add new paths to the list of existing paths. The shell will search through all the paths when searching for a command. 

```shell
|'L'|$ path /bin /usr/bin/
|'L'|$ 
```

Now the list of paths include both `/bin/` and `/usr/bin/`.

### Redirection

This shell supports basic redirection. If the file that the output of the redirection is pointing to already exist, it will be truncated and overwirtten with new content. 

*Note*: Multiple redirection is not supported. 

Example:

```shell
|'L'|$ ls -la
total 344
drwxr-xr-x   7 user  staff     224 Jun  8 22:19 .
drwxr-xr-x@  7 user  staff     224 Jun  8 21:38 ..
drwxr-xr-x  14 user  staff     448 Jun  8 21:45 .git
-rw-r--r--   1 user  staff     119 Jun  8 21:50 Makefile
-rw-r--r--@  1 user  staff    1269 Jun  8 22:19 README.md
-rwxr-xr-x   1 user  staff  144048 Jun  8 21:50 shell
-rw-r--r--@  1 user  staff   18525 Jun  8 21:51 shell.cpp
|'L'|$ ls -la > output.txt
|'L'|$ cat output.txt
total 344
drwxr-xr-x   8 user  staff     256 Jun  8 22:19 .
drwxr-xr-x@  7 user  staff     224 Jun  8 21:38 ..
drwxr-xr-x  14 user  staff     448 Jun  8 21:45 .git
-rw-r--r--   1 user  staff     119 Jun  8 21:50 Makefile
-rw-r--r--@  1 user  staff    1269 Jun  8 22:19 README.md
-rw-r--r--   1 user  staff       0 Jun  8 22:19 output.txt
-rwxr-xr-x   1 user  staff  144048 Jun  8 21:50 shell
-rw-r--r--@  1 user  staff   18525 Jun  8 21:51 shell.cpp
|'L'|$ 
```

### Parallel Commands

This shell also supports parallel commands for up to 3. 

This is simulated with `fork()` and `waitpid`in `c++`. The order of execution of the commands cannot be guaranteed.

Commands should be separated with `&`.

### Error

In most cases of error, the shell will continue and prints out the error message `An error has occurred`. 

For fatal errors such as cannot open files or passed wrong number of command line arguments, the shell will exist with `exit(1)`.

The error message of system commands such as `ls` or `pwd` will still be printed out. 

### Modes

There are two modes supported. 

- Interactive mode.

  - Similar to an actual Unix shell, just type the commands following the prompt `|'L'|$`.

  - ```shell
    $ ./shell
    |'L'|$ 
    ```

- Batch mode.

  - This mode will take in exactly one command line argument, a file. The file should contain a list of commands for the shell to execute, each occupying a single line. 
  - There will not be the cool `|'L'|$` printed in this mode. 

### Version

This is currently version 1.0. More features might be added in the future to further simulate a real Unix shell. 

### History

- June 8, 2021 - Initial version added to github. Initial README as well. 
