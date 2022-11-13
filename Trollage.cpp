#include <iostream>
#include <string_view>
#include <signal.h>
#include <filesystem>
#include <unistd.h>

#include <setjmp.h>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#include <readline/history.h>
#include <readline/readline.h>

#include <boost/process/search_path.hpp>
#include <boost/process/child.hpp>

#include <sys/types.h>
#include <pwd.h>

#include <regex>

sigjmp_buf ctrlc_buf;


struct passwd *pw = getpwuid(getuid());

const char *homedir = pw->pw_dir;

const std::string VerType = "[BETA]";
const std::string Version = "2";
const std::string HEADER   = "\033[95m";
const std::string BLUE     = "\033[94m";
const std::string CYAN     = "\033[96m";
const std::string GREEN    = "\033[92m";
const std::string WARNING  = "\033[93m";
const std::string FAIL     = "\033[91m";
const std::string ENDC     = "\033[0m";
const std::string BOLD     = "\033[1m";
const std::string UNDERLINE = "\033[4m";
const std::string PINK      = "\u001b[38;5;213m";
const std::string YELLOW    = "\u001b[38;5;228m";

void exiting(int signum) {
    std::cout << "Exiting..." << std::endl;
    exit(0);
}

void sigint(int sig){
	if (sig == SIGINT) {
		std::cout << "\n" << FAIL << BOLD << "[SIGINT] " << ENDC;
    		siglongjmp(ctrlc_buf, 1);
  	}
}

inline bool fileExist (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

void changeDir(std::string Comm){
    if (Comm.length() != 2){
        Comm = std::regex_replace(Comm, std::regex("~"), homedir);
	try {
            std::filesystem::current_path(Comm.substr(3,Comm.length()));
        } catch (std::filesystem::filesystem_error &e) {
            std::cout << BOLD << FAIL << "ERROR: " << WARNING << "Directory Not found: " << ENDC << Comm.substr(3,Comm.length()) << FAIL << BOLD << "\n[FAIL] ";
        }
    } else {
        std::cout << BOLD << FAIL << "ERROR: " << WARNING << "No Directory Entered" << FAIL << "\n[FAIL] ";
    }
}

void about() {
  std::cout << HEADER + BOLD + UNDERLINE << "Trollage Shell\n";
  std::cout << ENDC + BOLD << "Ver:" + ENDC << " " << CYAN << VerType << ENDC << " " << Version;
  std::cout << BOLD << "\nShell Colors:" + ENDC << " " + HEADER + "#" << ENDC + BLUE + "#" << CYAN + "#" << GREEN + "#" << WARNING + "#" << FAIL + "#" << ENDC + BOLD + "#" << ENDC + UNDERLINE + "#" << ENDC + PINK << "#" << YELLOW << "#" << ENDC << std::endl;
  std::cout << BOLD << "Made with " << PINK << "<3 " << ENDC + BOLD << "by: " << ENDC + YELLOW << "Lucy <3" << ENDC << std::endl;
}

std::string exe(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error(FAIL + "ERROR:" + WARNING + " Command Failed" + ENDC);
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}


int main(int argc, char** argv) {
    int interupted = 0;
    std::string Arg;

    if (argc > 1) {
        Arg = argv[1];
    } else {
        Arg = "none";
    }
    if (Arg == "--version"){
        std::cout << "Trollage Shell " << VerType << ENDC << " " << Version << std::endl;
        exit(0);
    } else if (Arg != "none"){
        std::cout << FAIL << BOLD << "ERROR: " << WARNING << Arg << " Is Not a Valid Command" << ENDC << std::endl;
        exit(0);
    }
    signal(SIGINT, sigint);
    signal(SIGSEGV, exiting);

    // configure readline to autocomplete when tab is pressed
    rl_bind_key('\t', rl_complete);
    std::cout << "Welcome to Trollage Shell " << CYAN << VerType << ENDC << " " << Version << "\nType " << CYAN + BOLD << "'exit' " << ENDC << "or press " << BOLD + GREEN << "'ctrl + d' " << ENDC << "to exit\n";
    while (true) {
        std::string CWD = std::filesystem::current_path();
	CWD = std::regex_replace(CWD, std::regex(homedir), "~");
	std::string Prompt = BOLD + CYAN + CWD + GREEN + " > " + ENDC;
	while ( sigsetjmp( ctrlc_buf, 1 ) != 0 );
        char* input = readline(Prompt.c_str());

        add_history(input);
        std::string_view input_view{input};

        if (input_view.compare("exit") == 0) {
            std::cout << "Exiting..." << std::endl;
            exit(0);
        } else if (input_view.substr(0, 2) == "cd") {
            changeDir(input);
        } else if (input_view.compare("about") == 0) {
            about();
        } else {
            std::string inputStr = input;
            std::string command = inputStr.substr(0,inputStr.find(" "));
            auto commandPath = boost::process::search_path(command);
            if (commandPath != "") {
                system(input);
            } else if (command.substr(0,2) == "./" or command.substr(0,2) == "~/" or command.substr(0,1) == "/") {
                	bool exist = fileExist(command);
             	if (exist == true) {
                    system(input);
                } else {
                    std::cout << BOLD << FAIL << "ERROR: " << WARNING << "No Such File Or Directory: " << ENDC << command << FAIL << BOLD << "\n[FAIL] ";
                }
            } else if (command.substr(0,1) == "$") {
		    system(command.c_str());
	    } else if (command == "") {
	    } else {
                std::cout << BOLD << FAIL << "ERROR: " << WARNING << "Command Not found: " << ENDC << command << FAIL << BOLD << "\n[FAIL] ";
            }
            //system(input);
            //std::string res = exe(input);
            //std::cout << res;
        }

        free(input);
    }

    return 0;
}
