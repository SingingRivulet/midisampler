#include "../melody2chord.h"
#include <sstream>
#include <algorithm>
#include <memory.h>
inline bool isNotHalfNote(int note,int base){
    const static bool l[] = {true,false,true,false,true,true,false,true,false,true,false,true};
    return l[(note-base+12)%12];
}

inline bool checkMajor(int note){
    const static bool l[] = {true,false,false,false,true,false,false,true,false,false,false,false};
    return l[note%12];
}
int getBaseTone(const std::string & notes){
    int single_note_count[12];//记录每个音符出现次数
    for(int i=0;i<12;++i){
        single_note_count[i] = 0;
    }
    int note_count = 0;

    std::istringstream iss(notes);
    while(iss.good()){
        std::string buf;
        iss>>buf;
        if(!buf.empty()){
            int note = atoi(buf.c_str());
            note_count ++;
            single_note_count[((note)+12)%12] ++;
        }else{
            break;
        }
    }

    int baseTone = 0;
    std::tuple<int, float> major_prob[12];
    for (int base = 0; base < 12; ++base) {
        int nh_count = 0;
        for(int i=0;i<12;++i){
            if (isNotHalfNote(i, base)) {
                nh_count += single_note_count[i];
            }
        }
        if (note_count == 0) {
            major_prob[base] = std::make_tuple(base, 0);
        } else {
            major_prob[base] = std::make_tuple(base, ((float) nh_count) / ((float) note_count));
        }
    }
    std::sort(major_prob, major_prob + 12,
              [](const std::tuple<int, float> &x, const std::tuple<int, float> &y) {
                  return std::get<1>(x) > std::get<1>(y);
              });
    if (checkMajor(std::get<1>(major_prob[0]))) {
        baseTone = std::get<0>(major_prob[0]);
    } else {
        if (std::get<1>(major_prob[1]) == std::get<1>(major_prob[0])) {
            baseTone = std::get<0>(major_prob[1]);
        } else {
            baseTone = std::get<0>(major_prob[0]);
        }
    }
    return baseTone;
}
const char * noteMap[] = {"1","1#","2","2#","3","4","4#","5","5#","6","6#","7"};
char buf_line[4096];
int main(){
    int index=0;
    auto fp = fopen("note-chord.txt","r");
    auto out = fopen("note-chord-crf.txt","w");
    mgnr::chordMap m;
    if(fp){
        while (!feof(fp) && index<150){
            bzero(buf_line,sizeof(buf_line));
            fgets(buf_line,sizeof(buf_line),fp);
            if(strlen(buf_line)<=2){
                ++index;
                fprintf(out,"\n");
                continue;
            }
            std::istringstream iss(buf_line);
            std::vector<int> notes;
            for(int i=0;i<8;++i){
                int note = 0;
                iss>>note;
                notes.push_back(note);
            }
            auto melody = mgnr::noteSeq2Chord(m,notes);
            int melody_note = std::get<0>(melody);
            fprintf(out,"%s %s ",melody_note<=0?"0":noteMap[melody_note%12],std::get<1>(melody).c_str());
            std::string chord,section;

            iss>>chord;
            iss>>section;
            
            fprintf(out,"%s ",section.c_str());
            fprintf(out,"%s\n",chord.c_str());
        }
        fclose(fp);
        fclose(out);
    }
}
