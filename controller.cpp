//
// Created by Marek on 27.12.2017.
//
#include <iostream>
#include <iomanip>
#include <vector>
#include <dirent.h>
#include <algorithm>
#include <thread>
#include <chrono>
#include <fstream>
#include <mutex>
#include <cmath>
#include <iomanip>
#include <map>
#include <future>

#include "controller.hpp"
#include "Command.cpp"

//map definition for word frequencies
typedef std::map<std::string, std::size_t> Words;

//templates definition for flipping around map keys and values - used to print word frequencies by frequency and not alphabetically
template<typename TimePoint>
std::chrono::milliseconds to_ms(TimePoint tp) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp);
}

template<typename A, typename B>
std::pair<B, A> flip_pair(const std::pair<A, B> &p) {
    return std::pair<B, A>(p.second, p.first);
}

template<typename A, typename B>
std::multimap<B, A> flip_map(const std::map<A, B> &src) {
    std::multimap<B, A> dst;
    std::transform(src.begin(), src.end(), std::inserter(dst, dst.begin()),
                   flip_pair<A, B>);
    return dst;
}


class Controller {

private:
    //working variables
    int thread_count = 1;
    std::string work_file = "";
    Command command = Command::CHANGE_INPUT;

    //handles commands
    void query() {
//        std::cout << "query" << std::endl;
        switch (ask_for_command()) {
            case Command::HELP:
                print_commands();
                return;
            case Command::INFO:
                print_info();
                return;
            case Command::QUIT:
                std::exit(0);
            case Command::WORD_COUNT:
                std::cout << "Total words: " << count_words_in_file() << std::endl;
                return;
            case Command::WORD_FREQUENCY_COUNT:
                run_frequency_count();
                return;
            case Command::RUN_TESTS:
                run_tests();
                return;
            case Command::RUN_THREAD_BENCHMARK:
                run_benchmark();
                return;
            case Command::CHANGE_INPUT:
                ask_for_file_selection();
                return;
            default:
                return;
        }


    }

    //asks for command input
    Command ask_for_command() {
        std::string input;

        std::cout << std::endl << "Using file '" << work_file.substr(10, work_file.length()) << "', working with "
                  << this->thread_count << " thread(s)" << std::endl;
        std::cout << "Please enter command" << std::endl;

        std::getline(std::cin, input);
//        std::cout << "Command entered: "<<input << std::endl;
        return interpret_command(input);
    }

    Command interpret_command(std::string input) {
//        std::cout << "Interpreting command: " << input << std::endl;
        std::transform(input.begin(), input.end(), input.begin(), ::tolower);
//        std::cout << "Lowercase: " << input << std::endl;
        if (input == "q") {
            return Command::QUIT;
        } else if (input == "help") {
            return Command::HELP;
        } else if (input == "info") {
            return Command::INFO;
        } else if (input == "cin") {
            return Command::CHANGE_INPUT;
        } else if (input == "tst") {
            return Command::RUN_TESTS;
        } else if (input == "tbc") {
            return Command::RUN_THREAD_BENCHMARK;
        } else if (input == "wc") {
            return Command::WORD_COUNT;
        } else if (input == "wf") {
            return Command::WORD_FREQUENCY_COUNT;
        } else if (input == "havlicek") {
            std::cout
                    << "Password accepted. Welcome Ing. Radek Havlicek, Ph.D., the almighty FEL scholar of great renown.";
            return ask_for_command();
        } else if (input == "t1") {
            this->thread_count = 1;
        } else if (input == "t2") {
            this->thread_count = 2;
        } else {
            std::cerr << "Command not recognised" << std::endl;
            return ask_for_command();
        }


    }

    //detects files in directory
    void get_files(const std::string &dir, std::vector<std::string> &files) {
        DIR *dp;
        struct dirent *dirp;
        if ((dp = opendir(dir.c_str())) == nullptr) {
            std::cout << "Error(" << errno << ") opening " << dir << std::endl;
            //return errno;
        }

        while ((dirp = readdir(dp)) != nullptr) {
            files.emplace_back(dirp->d_name);
        }
        closedir(dp);
    }

    //prints available files
    void print_input_files() {
        //set up directory
        std::string dir = std::string("../inputs");
        std::vector<std::string> files = std::vector<std::string>();

        get_files(dir, files);

        //prints
        std::cout << std::endl << "Available inputs:" << std::endl;
        for (auto &file : files) {
            if (file.substr(0, 1) != ".") {
                std::cout << file << ", ";
            }
        }
        std::cout << "\b\b " << std::endl << std::endl;
    }

    void ask_for_file_selection() {
        print_input_files();

        std::string input;
        std::cout << "Please select input file from list above." << std::endl;
        std::getline(std::cin, input);
//        std::cout << "Command entered: "<<input << std::endl;
        select_file(input);
    }

    //selects specified file to be designated as work file
    void select_file(const std::string &name) {

        std::string dir = std::string("../inputs");
        std::vector<std::string> files = std::vector<std::string>();

        get_files(dir, files);

        for (auto &file : files) {
            if (file.substr(0, 1) != ".") {
                if (name == file || (name + ".txt" == file)) {
                    load_file(file);
//                    std::cout<<"workfile now: "<<work_file<<std::endl;
                    return;
                }
            }

        }
        std::cout << "404 File not found. Please try again." << std::endl;
        ask_for_file_selection();

    }

    //makes the specified file a work file
    bool load_file(const std::string &file) {

//        std::cout << "Loading file from: " <<"../inputs/"<< file << std::endl;

        int line_num = 0;
        std::ifstream infile("../inputs/" + file);
        std::string line;


        while (std::getline(infile, line)) {
//            std::cout<<"Line ["<<line_num<<"] is: "<<line<<std::endl;
            line_num++;
        }


        work_file = "../inputs/" + file;
        std::cout << "File '" << work_file.substr(10, work_file.length()) << "' successfully loaded" << std::endl;
        return true;
    }

    //prints a file line by line - numbering lines, too!
    void print_file(){
        int line_num = 1;
        std::ifstream infile(work_file);
        std::string line;

        //print each line
        while (std::getline(infile, line)) {
            std::cout<<"["<<line_num<<"] | "<<line<<std::endl;
            line_num++;
        }
    }

    //counts all the words in a line
    int count_words_in_line(const std::string &line) {
        int counter = 0;
        bool processing_word = false;
        bool last_word_split = false;

        const char *input_ptr;
        input_ptr = line.c_str();

        //counts words - goes through all chars in line
        while (*input_ptr && *input_ptr != '\n' && *input_ptr != '\r') {
//        std::cout<<"Processing char '"<<*input_ptr<<"'"<<std::endl;
            //if in the middle of a word already, only detecting end
            if (processing_word) {
                if (*input_ptr == ' ' || *input_ptr == '.' || *input_ptr == ',') {
                    processing_word = false;
                }
            } else {
                //if not processing word, any symbol other than space means a start of a word
                if (*input_ptr != ' ') {
                    processing_word = true;
                    counter++;
//                std::cout<<"Added counter at '"<<*input_ptr<<"'"<<std::endl;
                }
            }
            input_ptr++;
        }


        //if this was last line, no adjustments, if it contained "\n", then move pointer before it
        input_ptr--;
        if (*input_ptr != '\0' && *input_ptr != '\n' && *input_ptr != '\r') {
            input_ptr++;
        }


        //checks if last word was split to next line
        if (*--input_ptr == '-') {
            if (*--input_ptr != ' ') {
                counter--;
                last_word_split = true; //could be used in multithread later, but deprecated
//            std::cout << "Word split at end of line detected" << std::endl;
            }
        }

        return counter;
    }

    //counts the number of words in a file
    int count_words_in_file() {
//        std::cout << "Counting file: '" << work_file << "'" << std::endl;

        int counter = 0;

        int line_num = 0;
        std::ifstream infile(work_file);
        std::string line;

        std::vector<std::string> lines;

        while (std::getline(infile, line)) {
            lines.push_back(line);
        }

        //in one threaded version, count line by line
        if (thread_count == 1) {
            for (const std::string &single_line:lines) {
//            std::cout << "Line [" << line_num << "] is: " << line << std::endl;
//            std::cout << "Total words in line [" << line_num << "]: " << count_words_in_line(line) << std::endl;

                counter += count_words_in_line(single_line);
                line_num++;

            }
        } else {
            //with two threads, it splits the lines to even and odd and counts separately
            std::mutex mutex;
            std::vector<std::thread> threads;
            auto thread_count_line = [&counter, &line, &mutex, this]() {
                mutex.lock();
                counter += count_words_in_line(line);
                mutex.unlock();
            };


            //lambda functions that count lines - odd
            int count1 = 0;
            bool odd = true;
            auto thread_count_odd = [&count1, &lines, &odd, &mutex, this]() {
                for (const std::string &single_line:lines) {
                    if (odd) {
//                    std::cout << "Count odd" << std::endl;

                        count1 += count_words_in_line(single_line);


                    }
                    odd = !odd;
                }


            };


            //lambda functions that count lines - even
            bool even = false;
            int count2 = 0;
            auto thread_count_even = [&count2, &lines, &even, &mutex, this]() {

                for (const std::string &single_line:lines) {
                    if (even) {
//                    std::cout << "Count even" << std::endl;
                        count2 += count_words_in_line(single_line);
                    }
                    even = !even;
                }

            };

            //separate counts
            std::thread t1(thread_count_odd);
            std::thread t2(thread_count_even);

            t1.join();
            t2.join();
            //add the line counts for odd and even
            counter = count1 + count2;

        }

        return counter;
    }

    //testing function
    void run_tests() {

        //backup working state
        int backup_t = this->thread_count;
        std::string backup_f = this->work_file;

        //test 1 - one word
        std::cout <<std::endl<< "Test 1: one word" << std::endl;

        load_file("test1.txt");
        std::cout << "---------- INPUT START ----------" << std::endl;
        print_file();
        std::cout << "----------- INPUT END -----------" << std::endl;
        std::cout << "Selected file '" << work_file.substr(10, work_file.length()) << "' has [" << count_words_in_file() << "] words" << std::endl;

        //test 2 - one line

        std::cout <<std::endl<< "Test 2: more words" << std::endl;

        load_file("test2.txt");
        std::cout << "---------- INPUT START ----------" << std::endl;
        print_file();
        std::cout << "----------- INPUT END -----------" << std::endl;
        std::cout << "Selected file '" << work_file.substr(10, work_file.length()) << "' has [" << count_words_in_file() << "] words" << std::endl;

        //test 3 - more lines

        std::cout <<std::endl<< "Test 3: more lines" << std::endl;

        load_file("test3.txt");
        std::cout << "---------- INPUT START ----------" << std::endl;
        print_file();
        std::cout << "----------- INPUT END -----------" << std::endl;
        std::cout << "Selected file '" << work_file.substr(10, work_file.length()) << "' has [" << count_words_in_file() << "] words" << std::endl;

        //test 4 - split words

        std::cout <<std::endl<< "Test 4: split words" << std::endl;

        load_file("test4.txt");
        std::cout << "---------- INPUT START ----------" << std::endl;
        print_file();
        std::cout << "----------- INPUT END -----------" << std::endl;
        std::cout << "Selected file '" << work_file.substr(10, work_file.length()) << "' has [" << count_words_in_file() << "] words" << std::endl;

        //thread test
        std::cout <<std::endl<< "Test 5: threads" << std::endl;

        load_file("test_big.txt");

        std::cout << std::endl << "1 thread:" << std::endl;
        thread_count = 1;
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "Selected file '" << work_file.substr(10, work_file.length()) << "' has [" << count_words_in_file() << "] words" << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        auto time1 = to_ms(end - start).count();
        std::cout << "Needed " << time1 << " ms to finish.\n";

        std::cout << std::endl << "2 threads:" << std::endl;
        thread_count = 2;
        auto start2 = std::chrono::high_resolution_clock::now();
        std::cout << "Selected file '" << work_file.substr(10, work_file.length()) << "' has [" << count_words_in_file() << "] words" << std::endl;
        auto end2 = std::chrono::high_resolution_clock::now();
        auto time2 = to_ms(end2 - start2).count();
        std::cout << "Needed " << time2 << " ms to finish.\n";

        float impro1 = time1 - time2;
        float impro2 = (impro1 / time1) * 100;

        std::cout << std::endl << "Measured improvement: " << impro1 << "ms/" << std::setprecision(3) << impro2 << "%"
                  << std::endl;

        this->work_file = backup_f;
        this->thread_count = backup_t;
    }

    //benchmark function
    void run_benchmark() {

        //backup working state
        int backup_t = this->thread_count;
        std::string backup_f = this->work_file;

        //variable setup
        const int num_tests = 10;
        float improvements[num_tests];


        load_file("test_big.txt");

        //table print
        std::cout << std::endl << std::left << std::setw(7) << "Test id" << "\t" << std::setw(8) << "1T time" << "\t"
                  << std::setw(8)
                  << "2T time" << "\t" << "Improvement" << std::endl;

        for (int i = 1; i <= num_tests; i++) {

            thread_count = 1;
            auto start = std::chrono::high_resolution_clock::now();
            count_words_in_file();
            auto end = std::chrono::high_resolution_clock::now();
            auto time1 = to_ms(end - start).count();
//            std::cout << "Needed " << time1 << " ms to finish.\n";

            thread_count = 2;
            auto start2 = std::chrono::high_resolution_clock::now();
            count_words_in_file();
            auto end2 = std::chrono::high_resolution_clock::now();
            auto time2 = to_ms(end2 - start2).count();
//            std::cout << "Needed " << time2 << " ms to finish.\n";

            float impro1 = time1 - time2;
            float impro2 = (impro1 / time1) * 100;

            improvements[i - 1] = impro2;

            std::cout << std::left << std::setw(7) << i << "\t" << time1 << std::setw(5) << "ms" << "\t" << time2
                      << std::setw(5) << "ms" << "\t" << std::setprecision(3) << impro2 << "%" << std::endl;
        }

        float average_improvement = 0;
        float average_deviation = 0;

        for (float improvement : improvements) {
            average_improvement += improvement;
        }
        average_improvement = average_improvement / num_tests;

        for (float improvement : improvements) {
//            std::cout<<"Dev count:"<<improvement<<"-"<<average_improvement<<std::endl;
            average_deviation += fabs(average_improvement - improvement);
//            std::cout<<"Dev count end:"<<average_deviation<<std::endl;
        }
        average_deviation = average_deviation / num_tests;

        //Final overview
        std::cout << "-------------- Overview --------------" << "" << std::endl;
        std::cout << std::setw(25) << "Average improvement:" << "\t" << std::setprecision(3) << average_improvement
                  << "%" << std::endl;
        std::cout << std::setw(25) << "Average deviation:" << "\t" << std::setprecision(3) << average_deviation << "%"
                  << std::endl;

        this->work_file = backup_f;
        this->thread_count = backup_t;
    }

    //counts frequencies of individual words
    void run_frequency_count() {
        std::vector<std::future<Words>> futures;

        //asynchronously load all word frequencies
        futures.push_back(std::async([=] { return words_in_file(work_file); }));

        Words words;

        for (auto &i : futures) {
            const auto results = i.get();

            for (const auto &j : results) {
                words[j.first] = j.second;
            }
        }

//    old - sorts alphabetically and not by frequency
//
//    std::cout << "Word\tRepeated Times\n-------------------------\n";
//    for (const auto &i : words) {
//        std::cout << std::left << std::setw(20) << i.first << "\t\t" << i.second << '\n';
//    }
//
//    std::cout<< std::setw(20)<<"Unique words: "<<words.size()<<std::endl;

        //flips values for keys
        std::multimap<std::size_t, std::string> sorted = flip_map(words);

        //prints the final table
        std::cout << std::setw(20) << "Word" << "\t" << "Count" << std::endl;
        for (const auto &i : sorted) {
            std::cout << std::left << std::setw(20) << i.second << "\t" << i.first << '\n';
        }

        std::cout << std::setw(20) << "Unique words: " <<"\t"<< sorted.size() << std::endl;
    }

    //counts individual words in file
    Words words_in_file(const std::string &fileName) {
        std::ifstream file(fileName);
        if (!file.is_open()) {
            std::cerr << "Can't open the file" << fileName;
        }

        //map containing words
        Words loadFromFile;


        for (std::string word; file >> word;) {
            // erase invalid chars
            word.erase(std::remove_if(word.begin(), word.end(), [](char c) {
                           return c == ',' || c == '.' || c == '!' || c == ':' || c == ')' || c == '(' || c == ';' || c == '\'' ||
                                  c == '/'|| c == '-';
                       }),
                       word.end());

            //change to lower case
            std::transform(word.begin(), word.end(), word.begin(), ::tolower);
//        std::cout << "Loading: '" << word << "'" << std::endl;
            ++loadFromFile[word];
        }

        return loadFromFile;
    }

public:

    //prints helpful commands
    void print_commands() {
        std::cout << std::left << std::setw(15) << "Command" << "\t" << "Description" << std::endl;

        //functionality
        std::cout << "-Functionality-" << "" << std::endl;
        std::cout << std::setw(15) << "wc" << "\t" << "Run total word count" << std::endl;
        std::cout << std::setw(15) << "wf" << "\t" << "Run word frequency count" << std::endl;

        //threads
        std::cout << "----Threads----" << "" << std::endl;
        std::cout << std::setw(15) << "t1" << "\t" << "Use a single thread" << std::endl;
        std::cout << std::setw(15) << "t2" << "\t" << "Use a two-threaded implementation" << std::endl;

        //testing
        std::cout << "----Testing----" << "" << std::endl;
        std::cout << std::setw(15) << "tst" << "\t" << "Run prepared tests" << std::endl;
        std::cout << std::setw(15) << "tbc" << "\t" << "Run multi-thread comparison benchmark" << std::endl;

        //other
        std::cout << "-----Other-----" << "" << std::endl;
        std::cout << std::setw(15) << "cin" << "\t" << "Change input text" << std::endl;
        std::cout << std::setw(15) << "help" << "\t" << "Prints possible commands" << std::endl;
        std::cout << std::setw(15) << "info" << "\t" << "Prints information about the program" << std::endl;
        std::cout << std::setw(15) << "q" << "\t" << "Exit program" << std::endl;

        //formatted text example
        //cout << "\033[;33m"<<"yellow text"<<"\033[0m\n";
    }

    //prints info
    void print_info() {
        std::cout << "----- Info -----" << "" << std::endl;
        std::cout << "This WordCount program was created by CTU student Marek Szeles between Nov 2017-Jan 2018 as a semestral project for the B6B36PJC subject" << std::endl;
    }

    //starts instance
    void start() {
//        std::cout << "Controller start success" << "" << std::endl;
        while (command != Command::QUIT) {
//            std::cout << "Controller loop query" << "" << std::endl;
            if (work_file.empty()) {
                ask_for_file_selection();
            } else {
                query();
            }
        }
    }

};