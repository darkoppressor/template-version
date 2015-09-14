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
    cout<<program_name<<" - increment a component of a Cheese Engine project's version number\n";
    cout<<"Usage: "<<program_name<<" PROJECT-DIRECTORY VERSION-COMPONENT STATUS\n";
    cout<<"VERSION-COMPONENT must be one of: 'major' 'minor' 'micro'\n";
    cout<<"STATUS is optional and can be any string\n";
}

int main(int argc,char* args[]){
    File_IO file_io;
    String_Stuff string_stuff;

    //Can this even happen?
    if(argc<=0){
        print_error("Did not receive the program name");

        return 1;
    }
    else if(argc<3 || argc>4){
        print_usage(args[0]);

        return 0;
    }

    string project_directory=args[1];

    if(project_directory.length()==0){
        print_error("The PROJECT-DIRECTORY argument has a length of 0");

        return 1;
    }

    if(boost::algorithm::ends_with(project_directory,"/") || boost::algorithm::ends_with(project_directory,"\\")){
        project_directory.erase(project_directory.begin()+project_directory.length()-1);
    }

    if(!boost::filesystem::is_directory(project_directory)){
        print_error("No such directory: "+project_directory);

        return 1;
    }

    string version_component=args[2];

    if(version_component!="major" && version_component!="minor" && version_component!="micro"){
        print_usage(args[0]);

        return 0;
    }

    string status="";

    if(argc==4){
        status=args[3];
    }

    Version version_old=get_version(project_directory);
    Version version_new=version_old;

    if(version_new.major<0 || version_new.minor<0 || version_new.micro<0){
        return 1;
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

    if(!update_version_header(project_directory,version_new,status)){
        return 1;
    }

    if(!update_info_plist(project_directory,version_old,version_new)){
        return 1;
    }

    if(!update_android_manifest(project_directory,version_old,version_new)){
        return 1;
    }

    return 0;
}

Version get_version(string project_directory){
    Version version;

    if(!boost::filesystem::exists(project_directory+"/version.h")){
        print_error("No such file: "+project_directory+"/version.h");

        return version;
    }

    cout<<"Reading current version number for the project in "<<project_directory<<"\n";

    String_Stuff string_stuff;

    ifstream file(project_directory+"/version.h");

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

bool update_version_header(string project_directory,const Version& version,const string& status){
    if(!boost::filesystem::exists(project_directory+"/version.h")){
        print_error("No such file: "+project_directory+"/version.h");

        return false;
    }

    cout<<"Incrementing the version.h version number for the project in "<<project_directory<<"\n";

    vector<string> file_data;

    String_Stuff string_stuff;

    ifstream file(project_directory+"/version.h");

    if(file.is_open()){
        while(!file.eof()){
            string line="";

            getline(file,line);

            file_data.push_back(line);
        }
    }
    else{
        print_error("Failed to open version.h for updating (input phase)");

        file.close();
        file.clear();

        return false;
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

    ofstream file_save(project_directory+"/version.h");

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

        file_save.close();
        file_save.clear();

        return false;
    }

    file_save.close();
    file_save.clear();

    return true;
}

bool update_info_plist(string project_directory,const Version& version_old,const Version& version_new){
    if(!boost::filesystem::is_directory(project_directory+"/development")){
        print_error("No such directory: "+project_directory+"/development");

        return false;
    }

    cout<<"Incrementing the Info.plist version number for the project in "<<project_directory<<"\n";

    String_Stuff string_stuff;

    string app_directory="";

    for(File_IO_Directory_Iterator it(project_directory+"/development/");it.evaluate();it.iterate()){
        if(it.is_directory()){
            string file_name=it.get_file_name();

            if(boost::algorithm::ends_with(file_name,".app")){
                app_directory=file_name;

                break;
            }
        }
    }

    if(app_directory.length()>0){
        string info_plist=project_directory+"/development/"+app_directory+"/Contents/Info.plist";

        string str_old_version=string_stuff.num_to_string(version_old.major)+"."+string_stuff.num_to_string(version_old.minor)+"."+string_stuff.num_to_string(version_old.micro);
        string str_new_version=string_stuff.num_to_string(version_new.major)+"."+string_stuff.num_to_string(version_new.minor)+"."+string_stuff.num_to_string(version_new.micro);

        if(!replace_in_file(info_plist,str_old_version,str_new_version)){
            return false;
        }
    }
    else{
        print_error("Failed to locate .app directory");

        return false;
    }

    return true;
}

bool update_android_manifest(string project_directory,const Version& version_old,const Version& version_new){
    if(!boost::filesystem::exists(project_directory+"/development/android/AndroidManifest.xml")){
        print_error("No such file: "+project_directory+"/development/android/AndroidManifest.xml");

        return false;
    }

    cout<<"Incrementing the AndroidManifest.xml version number for the project in "<<project_directory<<"\n";

    String_Stuff string_stuff;

    string str_old_version=string_stuff.num_to_string(version_old.major)+"."+string_stuff.num_to_string(version_old.minor)+"."+string_stuff.num_to_string(version_old.micro);
    string str_new_version=string_stuff.num_to_string(version_new.major)+"."+string_stuff.num_to_string(version_new.minor)+"."+string_stuff.num_to_string(version_new.micro);

    if(!replace_in_file(project_directory+"/development/android/AndroidManifest.xml",str_old_version,str_new_version)){
        return false;
    }

    vector<string> file_data;

    ifstream file(project_directory+"/development/android/AndroidManifest.xml");

    if(file.is_open()){
        while(!file.eof()){
            string line="";

            getline(file,line);

            file_data.push_back(line);
        }
    }
    else{
        print_error("Failed to open AndroidManifest.xml for updating (input phase)");

        file.close();
        file.clear();

        return false;
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

    ofstream file_save(project_directory+"/development/android/AndroidManifest.xml");

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

        file_save.close();
        file_save.clear();

        return false;
    }

    file_save.close();
    file_save.clear();

    return true;
}

bool replace_in_file(string filename,string target,string replacement){
    if(!boost::filesystem::exists(filename)){
        print_error("No such file: "+filename);

        return false;
    }

    cout<<"Renaming all occurrences of "<<target<<" to "<<replacement<<" in "<<filename<<"\n";

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

        file.close();
        file.clear();

        return false;
    }

    file.close();
    file.clear();

    for(int i=0;i<file_data.size();i++){
        boost::algorithm::replace_all(file_data[i],target,replacement);
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

        file_save.close();
        file_save.clear();

        return false;
    }

    file_save.close();
    file_save.clear();

    return true;
}
