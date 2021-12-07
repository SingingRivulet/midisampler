#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
bool checkChord(const std::vector<int> & chord){
    int last = -9999;
    for(auto note:chord){
        if(std::abs(note-last)<=2){
            return false;
        }
        last = note;
    }
    return true;
}
bool checkChord(const std::string & str){
    std::istringstream iss(str);
    std::string chordstr;
    int passchordnum=0;
    int chordnum=0;
    while (iss.good()){
        chordstr.clear();
        std::getline(iss,chordstr,']');
        if(chordstr.empty()){
            break;
        }
        int len = chordstr.size();
        for(int i=0;i<len;++i){
            char & c = chordstr[i];
            if(c==',' || c=='[' || c==']'){
                c = ' ';
            }
        }
        std::istringstream iss_chord(chordstr);
        std::vector<int> chord;
        while (iss_chord.good()){
            int note = 0;
            iss_chord>>note;
            if(note!=0){
                chord.push_back(note);
                //printf("%d ",note);
            }
        }
        //std::cout<<"data:"<<chordstr<<std::endl;
        if(checkChord(chord)){
            ++passchordnum;
            //printf(" true");
        }else{
            std::cout<<"error:";
            for(auto it:chord){
                std::cout<<it<<" ";
            }
            std::cout<<std::endl;
        }
        //printf("\n");
        ++chordnum;
    }
    if(chordnum<=0){
        return true;
    }

    float prob = ((float)passchordnum/(float)chordnum);
    //std::cout<<"passchordnum:"<<passchordnum<<" chordnum:"<<chordnum<<" prob:"<<prob<<std::endl;
    return prob>0.95;
}
bool checkChordLine(const std::string & str){
    std::istringstream iss(str);
    std::string chordstr;
    std::getline(iss,chordstr,'|');
    std::getline(iss,chordstr);
    return checkChord(chordstr);
}
char buf[65536];
int main(int argc, char** argv){
    if(argc<3){
        return 0;
    }
    auto in = fopen(argv[1],"r");
    auto out = fopen(argv[2],"w");
    if(in && out){
        while(!feof(in)){
            memset(buf,0,sizeof(buf));
            fgets(buf,sizeof(buf),in);
            if(checkChordLine(buf)){
                fprintf(out,"%s",buf);
            }
        }
    }
    if(in){
        fclose(in);
    }
    if(out){
        fclose(out);
    }
}