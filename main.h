/* Copyright (c) 2012 Cheese and Bacon Games, LLC */
/* This file is licensed under the MIT License. */
/* See the file docs/LICENSE.txt for the full license text. */

#ifndef main_h
#define main_h

#include <vector>
#include <string>

class Version{
public:

    int major;
    int minor;
    int micro;

    Version();

    void increment_major();
    void increment_minor();
    void increment_micro();
};

void print_error(std::string error_message);

void print_usage(std::string program_name);

int main(int argc,char* args[]);

Version get_version();

void update_version_header(const Version& version,const std::string& status);

void update_info_plist(const Version& version_old,const Version& version_new);

void update_android_manifest(const Version& version_old,const Version& version_new);

void rename_file(std::string target,std::string replacement);

void replace_in_file(std::string filename,std::string target,std::string replacement);

#endif
