/* Copyright (c) 2012 Cheese and Bacon Games, LLC */
/* This file is licensed under the MIT License. */
/* See the file docs/LICENSE.txt for the full license text. */

#include "main.h"
#include "string_stuff.h"
#include "file_io.h"

#include <iostream>
#include <vector>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

Version::Version(){
    major=-1;
    minor=-1;
    micro=-1;
}

void Version::increment_major(){
    major++;
    minor=0;
    micro=0;
}

void Version::increment_minor(){
    minor++;
    micro=0;
}

void Version::increment_micro(){
    micro++;
}

void print_error(string error_message){
    cout<<"Error: "<<error_message<<"\n";
}

void print_usage(string program_name){
    cout<<"Usage: "<<program_name<<" [VERSION-COMPONENT] [STATUS]\n";
    cout<<"VERSION-COMPONENT must be one of: 'major' 'minor' 'micro'\n";
    cout<<"[STATUS] is optional and can be any string\n";
}

int main(int argc,char* args[]){
    File_IO file_io;
    String_Stuff string_stuff;

    //Can this even happen?
    if(argc<=0){
        cout<<"Error: Did not receive the program name\n";

        return 1;
    }
    else if(argc==1){
        print_usage(args[0]);

        return 0;
    }

    string version_component=args[1];

    if(version_component!="major" && version_component!="minor" && version_component!="micro"){
        print_usage(args[0]);

        return 0;
    }

    string status="";

    if(argc>=3){
        status=args[2];
    }

    Version version_old=get_version();
    Version version_new=version_old;

    if(version_new.major<0 || version_new.minor<0 || version_new.micro<0){
        print_error("Failed to load version information from version.h");

        return 2;
    }

    if(version_component=="major"){
        version_new.increment_major();
    }
    else if(version_component=="minor"){
        version_new.increment_minor();
    }
    else if(version_component=="micro"){
        version_new.increment_micro();
    }

    update_version_header(version_new,status);

    update_info_plist(version_old,version_new);

    update_android_manifest(version_old,version_new);

    return 0;
}

Version get_version(){
    Version version;

    String_Stuff string_stuff;

    ifstream file("../version.h");

    if(file.is_open()){
        while(!file.eof()){
            string line="";

            getline(file,line);

            boost::algorithm::trim(line);

            if(boost::algorithm::contains(line,"static const int MAJOR=")){
                boost::algorithm::erase_first(line,"static const int MAJOR=");
                boost::algorithm::erase_first(line,";");
                boost::algorithm::trim(line);

                version.major=string_stuff.string_to_long(line);
            }
            else if(boost::algorithm::contains(line,"static const int MINOR=")){
                boost::algorithm::erase_first(line,"static const int MINOR=");
                boost::algorithm::erase_first(line,";");
                boost::algorithm::trim(line);

                version.minor=string_stuff.string_to_long(line);
            }
            else if(boost::algorithm::contains(line,"static const int MICRO=")){
                boost::algorithm::erase_first(line,"static const int MICRO=");
                boost::algorithm::erase_first(line,";");
                boost::algorithm::trim(line);

                version.micro=string_stuff.string_to_long(line);
            }
        }
    }
    else{
        print_error("Failed to open version.h for reading version information");
    }

    file.close();
    file.clear();

    return version;
}

void update_version_header(const Version& version,const string& status){
    vector<string> file_data;

    String_Stuff string_stuff;

    ifstream file("../version.h");

    if(file.is_open()){
        while(!file.eof()){
            string line="";

            getline(file,line);

            file_data.push_back(line);
        }
    }
    else{
        print_error("Failed to open version.h for updating (input phase)");
    }

    file.close();
    file.clear();

    for(int i=0;i<file_data.size();i++){
        if(boost::algorithm::contains(file_data[i],"const int")){
            for(int n=0;n<file_data[i].length();n++){
                if(file_data[i][n]=='='){
                    string component="";
                    if(boost::algorithm::contains(file_data[i],"MAJOR")){
                        component=string_stuff.num_to_string(version.major);
                    }
                    else if(boost::algorithm::contains(file_data[i],"MINOR")){
                        component=string_stuff.num_to_string(version.minor);
                    }
                    else{
                        component=string_stuff.num_to_string(version.micro);
                    }

                    file_data[i].erase(file_data[i].begin()+n,file_data[i].end());

                    file_data[i]+="="+component+";";

                    break;
                }
            }
        }
        else if(status.length()>0 && boost::algorithm::contains(file_data[i],"STATUS")){
            for(int n=0;n<file_data[i].length();n++){
                if(file_data[i][n]=='='){
                    file_data[i].erase(file_data[i].begin()+n,file_data[i].end());

                    file_data[i]+="=\""+status+"\";";

                    break;
                }
            }
        }
    }

    ofstream file_save("../version.h");

    if(file_save.is_open()){
        for(int i=0;i<file_data.size();i++){
            file_save<<file_data[i];

            if(i<file_data.size()-1){
                file_save<<"\n";
            }
        }
    }
    else{
        print_error("Failed to open version.h for updating (output phase)");
    }

    file_save.close();
    file_save.clear();
}

void update_info_plist(const Version& version_old,const Version& version_new){
    String_Stuff string_stuff;

    string app_directory="";

    for(File_IO_Directory_Iterator it("./");it.evaluate();it.iterate()){
        if(it.is_directory()){
            string file_name=it.get_file_name();

            if(boost::algorithm::contains(file_name,".app")){
                app_directory=file_name;

                break;
            }
        }
    }

    if(app_directory.length()>0){
        string info_plist=app_directory+"/Contents/Info.plist";

        string str_old_version=string_stuff.num_to_string(version_old.major)+"."+string_stuff.num_to_string(version_old.minor)+"."+string_stuff.num_to_string(version_old.micro);
        string str_new_version=string_stuff.num_to_string(version_new.major)+"."+string_stuff.num_to_string(version_new.minor)+"."+string_stuff.num_to_string(version_new.micro);

        replace_in_file(info_plist,str_old_version,str_new_version);
    }
    else{
        print_error("Failed to locate .app directory");
    }
}

void update_android_manifest(const Version& version_old,const Version& version_new){
    String_Stuff string_stuff;

    string str_old_version=string_stuff.num_to_string(version_old.major)+"."+string_stuff.num_to_string(version_old.minor)+"."+string_stuff.num_to_string(version_old.micro);
    string str_new_version=string_stuff.num_to_string(version_new.major)+"."+string_stuff.num_to_string(version_new.minor)+"."+string_stuff.num_to_string(version_new.micro);

    replace_in_file("android/AndroidManifest.xml",str_old_version,str_new_version);

    vector<string> file_data;

    ifstream file("android/AndroidManifest.xml");

    if(file.is_open()){
        while(!file.eof()){
            string line="";

            getline(file,line);

            file_data.push_back(line);
        }
    }
    else{
        print_error("Failed to open AndroidManifest.xml for updating (input phase)");
    }

    file.close();
    file.clear();

    for(int i=0;i<file_data.size();i++){
        if(boost::algorithm::contains(file_data[i],"android:versionCode")){
            string get_version_code=file_data[i];

            boost::algorithm::erase_first(get_version_code,"android:versionCode=\"");
            boost::algorithm::erase_first(get_version_code,"\"");
            boost::algorithm::trim(get_version_code);

            uint32_t version_code=string_stuff.string_to_unsigned_long(get_version_code);

            for(int n=0;n<file_data[i].length();n++){
                if(file_data[i][n]=='"'){
                    file_data[i].erase(file_data[i].begin()+n,file_data[i].end());

                    file_data[i]+="\""+string_stuff.num_to_string(++version_code)+"\"";

                    break;
                }
            }
        }
    }

    ofstream file_save("android/AndroidManifest.xml");

    if(file_save.is_open()){
        for(int i=0;i<file_data.size();i++){
            file_save<<file_data[i];

            if(i<file_data.size()-1){
                file_save<<"\n";
            }
        }
    }
    else{
        print_error("Failed to open AndroidManifest.xml for updating (output phase)");
    }

    file_save.close();
    file_save.clear();
}

void rename_file(string target,string replacement){
    boost::filesystem::rename(target,replacement);
}

void replace_in_file(string filename,string target,string replacement){
    vector<string> file_data;

    ifstream file(filename.c_str());

    if(file.is_open()){
        while(!file.eof()){
            string line="";

            getline(file,line);

            file_data.push_back(line);
        }
    }
    else{
        print_error("Failed to open "+filename+" for updating (input phase)");
    }

    file.close();
    file.clear();

    for(int i=0;i<file_data.size();i++){
        boost::algorithm::replace_first(file_data[i],target,replacement);
    }

    ofstream file_save(filename.c_str());

    if(file_save.is_open()){
        for(int i=0;i<file_data.size();i++){
            file_save<<file_data[i];

            if(i<file_data.size()-1){
                file_save<<"\n";
            }
        }
    }
    else{
        print_error("Failed to open "+filename+" for updating (output phase)");
    }

    file_save.close();
    file_save.clear();
}
