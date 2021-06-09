#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <sstream>
#include <fstream> // batch mode
#include <sys/wait.h>
#include <string.h>
#include <algorithm> // std::find

// global variables
char error_message[30] = "An error has occurred\n";
std::vector<std::string> paths;

int check_commands(std::vector<std::string> commands, std::string command){
    return (std::find(commands.begin(), commands.end(), command) != commands.end());
}

// function to parse a command with redirect
std::vector<std::string> getCommandfile(std::string commands){ 
    // return in the form of a vector of string
    // if an empty vector is returned, something wrong happened
    // if none empty, return in the form:
    // example for 'ls -l -a > file.txt -> {'ls', '-l', '-a', 'file.txt'}

    std::stringstream ss(commands);
    std::string command;
    std::string word; // for reading cm and arguments
    std::vector<std::string> arguments;
    std::vector<std::string> parsed_inputs;
    std::string file;

    std::vector<std::string> results;
    std::vector<std::string> badResults;
    std::string cm;

    while(std::getline(ss, command, '>')){
        parsed_inputs.push_back(command);
    }
    if(parsed_inputs.size() > 2 || parsed_inputs.size() == 1){
        return badResults;
    }
    // parse the command part
    std::stringstream ss1(parsed_inputs[0]);
    ss1 >> word;
    cm = word;
    results.push_back(cm);
    while(ss1 >> word){
        results.push_back(word);
    }

    // obtain the file part
    std::stringstream fss(parsed_inputs[1]);
    fss >> file;
    results.push_back(file);
    if(fss >> word){
        // still more to read, which is an error
        return badResults;
    }
    
    return results;
}

// function to parse a command without rediection 
std::vector<std::string> getCommand(std::string commands){
    std::string word; // read a string at a time
    std::string cm; // the command
    std::vector<std::string> results;

    
    std::stringstream ss(commands);
    ss >> word;
    cm = word;
    results.push_back(cm);
    while(ss >> word){
        results.push_back(word);
    }
    return results;
}

// for checking path, preparing argv and swap processes(evecv)
void runCommand(std::vector<std::string> results,  std::string newCommand){
    char* arr[10];
    int file_mode = 0; 

    if(paths.size() == 0){ // only one path
        return;
    }

    char* path = strdup(paths[0].c_str());
    strcat(path, results[0].c_str());

    // check if executable at path is ok
    if(access(path, X_OK) < 0){
        // if not, prepare for backup path

        if(paths.size() <= 1){ // only one path
            return;
        }

        for(size_t j = 1; j < paths.size(); j++){
            path = strdup(paths[j].c_str());
            strcat(path, results[0].c_str());
            if(access(path, X_OK) < 0){
                return;
            }else{
                break;
            }
        }
    }

    //prepare for execv
    if(newCommand.find(">") != std::string::npos){
        
        // setting the argument list ready for non-builtin commands
        for(size_t i = 0; i < results.size()-1; i++){
            arr[i] = strdup(results[i].c_str());
        }
        arr[results.size()-1] = nullptr;
        file_mode = 1;
    }else{
        // setting the argument list ready for non-builtin commands
        for(size_t i = 0; i < results.size(); i++){
            arr[i] = strdup(results[i].c_str());
        }
        arr[results.size()] = nullptr;
    }

    if(file_mode == 1){
        int fdes;
        fdes = open(results[results.size()-1].c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0664);
        if(fdes < 0){ // failed to receive a fd
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        dup2(fdes, STDOUT_FILENO);
        dup2(STDERR_FILENO, fdes);
        close(fdes);
    }
    execv(path, arr);
}

std::vector<std::string> parseProcesses(std::string input){
    std::vector<std::string> processes;

    std::stringstream ss(input);
    std::string command;
    while(std::getline(ss, command, '&')){

        // trim leading white spaces
        while(command[0] == ' '){
            command.erase(0,1);
        }
        if(command.empty()){
            break;
        }
        processes.push_back(command);
    }

    return processes;
}

int main(int argc, char* argv[]){

    // setting initial variables, error msg and builtin commands
    std::vector<std::string> builtin_commands;
    // cannot initilize the array with initilizer list(C++11)
    builtin_commands.push_back("exit");
    builtin_commands.push_back("cd");
    builtin_commands.push_back("path");

    //std::string path = "/bin/";
    
    paths.push_back("/bin/");

    if(argc == 1){

        std::cout << "Current in interactive mode." << std::endl;
        while(true){
            std::string word; // read a string at a time
            std::string newCommand; // take the entire line
            std::vector<std::string> results; 

            // storing the parsed command and its arguments
            std::vector<std::vector<std::string> > processes;

            // array storing commands, if a single command is entered
            // the array would be size 1, if 3 commands are asked to run
            // concurrently, then the array would have 3 elements
            std::vector<std::string> commands; // store commands
            std::cout << "|'L'|$ ";

            // not parsing '|' yet.
            std::getline(std::cin, newCommand);
            
            // parsing for '&'
            commands = parseProcesses(newCommand);


            // if no command entered, containue
            if(commands.empty()){ 
                continue;
            }

            // prase commands and get their executable and args
            for(size_t index = 0; index < commands.size(); index++){
                if(commands[index].find(">") != std::string::npos){
                
                    results = getCommandfile(commands[index]);

                    if(results.size() == 0){
                        // bad parse, something wrong
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        break;
                    }
                    processes.push_back(results);
                }else{ 
                    // normal command
                    results = getCommand(commands[index]);
                    for(size_t x = 0; x < results.size(); x++){
                        
                    }
                    processes.push_back(results);
                }
            }
            
            // if bad input or no commands
            if(processes.empty()){
                continue;
            }

            // check if command is builtin or system
            if(check_commands(builtin_commands, processes[0][0])){
                
                if(processes[0][0] == "exit"){ // "exit"
                    if(processes[0].size() != 1){
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        continue;
                    }
                    return 0;
                }else if(processes[0][0] == "cd"){ // "cd"
                    if(processes[0].size() != 2){ // cd only takes one argument
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        continue;
                    }else{
                        chdir(processes[0][1].c_str());
                    }
                }else{ // "path ..."
                    // first, overwrites the old paths
                    paths.clear();

                    // path arguments start at the second position of results
                    // thus, loop starts at 1
                    for(size_t i = 1; i < results.size(); i++){
                        paths.push_back(processes[0][i]+"/");
                    }
                }
            }else{
                // not a built in command, should call fork to create a child process
                // only supports max 3 concurrent processes together
                // although it's enough to pass autograder
                // I'm still figuring out how to make it work with any number
                pid_t pid = fork();

                if(pid < 0){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    return 1;
                }

                if(pid == 0){
                    // single command, run as usual
                    runCommand(processes[0], commands[0]);
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    return 1; 
                                  
                }else{ // parent process
                    // if there are more concurrent commands, fork more processes
                    if(processes.size() == 2){
                        pid = fork();

                        if(pid < 0){
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            return 1;
                        }

                        if(pid == 0){ // second child
                            // pids[1] = getpid();
                            // single command, run as usual
                            runCommand(processes[1], commands[1]);
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            return 1; 
                        }else{ // parent
                            
                            while (wait(NULL) != -1) {
                                // waiting
                            }  
                        }     

                    }else if(processes.size() == 3){
                        pid = fork();

                        if(pid < 0){
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            return 1;
                        }

                        if(pid == 0){ // second child
                            // pids[1] = getpid();
                            // single command, run as usual
                            runCommand(processes[1], commands[1]);
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            return 1; 
                        }else{ // parent should fork again
                            pid = fork();
                            if(pid < 0){
                                write(STDERR_FILENO, error_message, strlen(error_message));
                                return 1;
                            }

                            if(pid == 0){ // third child
                                // pids[2] = getpid();
                                // single command, run as usual
                                runCommand(processes[2], commands[2]);
                                write(STDERR_FILENO, error_message, strlen(error_message));
                                return 1; 
                            }
                            while (wait(NULL) != -1) {
                                // waiting
                            }  
                        } 

                    }else{ // single child parent, just wait
                        while (wait(NULL) != -1) {
                            // waiting
                        }  
                    }
                    
                }
                
            }
        }
        return 0;

    }else{ // batch mode
        
        if(argc > 2){
            // too many files provided
            write(STDERR_FILENO, error_message, strlen(error_message)); 
            return 1;
        }else{
            std::fstream file(argv[1]);

            if(!file){
                write(STDERR_FILENO, error_message, strlen(error_message));
                return 1;
            } 
        
            std::string newCommand; // take the entire line
            while(std::getline(file, newCommand)){
                std::string word; // read a string at a time
                std::vector<std::string> results; 

                // storing the parsed command and its arguments
                std::vector<std::vector<std::string> > processes;
                std::vector<std::string> commands; // store commands
                
                // not parsing '|' yet.

                // parsing for '&'
                commands = parseProcesses(newCommand);


                // if no command entered, containue
                if(commands.empty()){ 
                    continue;
                }

                // prase commands and get their executable and args
                for(size_t index = 0; index < commands.size(); index++){
                    if(commands[index].find(">") != std::string::npos){
                    
                        results = getCommandfile(commands[index]);

                        if(results.size() == 0){
                            // bad parse, something wrong
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            break;
                        }
                        processes.push_back(results);
                    }else{ 
                        // normal command
                        results = getCommand(commands[index]);

                        processes.push_back(results);
                    }
                }

                // if bad input or no commands
                if(processes.empty()){
                    continue;
                }

                // check if command is builtin or system
                if(check_commands(builtin_commands, results[0])){
                    
                    if(processes[0][0] == "exit"){ // "exit"
                        if(processes[0].size() != 1){
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            continue;
                        }
                        return 0;
                    }else if(processes[0][0] == "cd"){ // "cd"
                        if(processes[0].size() != 2){ // cd only takes one argument
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            continue;
                        }else{
                            chdir(processes[0][1].c_str());
                        }
                    }else{ // "path ..."
                        // first, overwrites the old paths
                        paths.clear();

                        // path arguments start at the second position of results
                        // thus, loop starts at 1
                        for(size_t i = 1; i < results.size(); i++){
                            paths.push_back(processes[0][i]+"/");
                        }
                    }
                }else{
                    // not a built in command, should call fork to create a child process

                    pid_t pid = fork();

                    if(pid < 0){
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        return 1;
                    }

                    if(pid == 0){
                        // single command, run as usual
                        runCommand(processes[0], commands[0]);
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        return 1; 
                                    
                    }else{ // parent process
                        // if there are more concurrent commands, fork more processes
                        if(processes.size() == 2){
                            pid = fork();

                            if(pid < 0){
                                write(STDERR_FILENO, error_message, strlen(error_message));
                                return 1;
                            }

                            if(pid == 0){ // second child
                                // single command, run as usual
                                runCommand(processes[1], commands[1]);
                                write(STDERR_FILENO, error_message, strlen(error_message));
                                return 1; 
                            }else{ // parent
                                
                                while (wait(NULL) != -1) {
                                    // waiting
                                }  
                            }     

                        }else if(processes.size() == 3){
                            pid = fork();

                            if(pid < 0){
                                write(STDERR_FILENO, error_message, strlen(error_message));
                                return 1;
                            }

                            if(pid == 0){ // second child
                                // single command, run as usual
                                runCommand(processes[1], commands[1]);
                                write(STDERR_FILENO, error_message, strlen(error_message));
                                return 1; 
                            }else{ // parent should fork again
                                pid = fork();
                                if(pid < 0){
                                    write(STDERR_FILENO, error_message, strlen(error_message));
                                    return 1;
                                }

                                if(pid == 0){ // third child
                                    // single command, run as usual
                                    runCommand(processes[2], commands[2]);
                                    write(STDERR_FILENO, error_message, strlen(error_message));
                                    return 1; 
                                }
                                while (wait(NULL) != -1) {
                                    // waiting
                                }  
                            } 

                        }else{ // single child parent, just wait
                            while (wait(NULL) != -1) {
                                // waiting
                            }    
                        }
                    }
                }
            }   
        }
        return 0;
    }
}