/* Copyright (c) 2012 Cheese and Bacon Games, LLC */
/* This file is licensed under the MIT License. */
/* See the file docs/LICENSE.txt for the full license text. */

#ifndef main_h
#define main_h

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

Version get_version(std::string project_directory);

bool update_version_file(std::string project_directory,const Version& version,const std::string& status);

bool update_info_plist(std::string project_directory,const Version& version_old,const Version& version_new);

bool update_android_manifest(std::string project_directory,const Version& version_old,const Version& version_new);

bool replace_in_file(std::string filename,std::string target,std::string replacement);

#endif
