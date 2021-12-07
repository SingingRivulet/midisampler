#include <sstream>
#include <algorithm>
#include <tuple>
#include <string>
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
char buf_line[65526];
const char * noteMap[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
int main(int argc, char** argv){
    if(argc<2){
        return 0;
    }
    auto fp = fopen(argv[1],"r");
    int index = 0;
    if(fp){
        while(!feof(fp)){
            ++index;
            bzero(buf_line,sizeof(buf_line));
            fgets(buf_line,sizeof(buf_line),fp);
            int len = strlen(buf_line);
            if(len<=1){
                continue;
            }
            for(int i=0;i<len;++i){
                char & c = buf_line[i];
                if(c==',' || c=='[' || c==']'){
                    c = ' ';
                }else if(c=='|'){
                    c = '\0';
                    break;
                }
            }
            int tone = getBaseTone(buf_line);
            printf("行号：%d 调性：%d(%s)\n",index,tone,noteMap[tone%12]);
        }
        fclose(fp);
    }
    return 0;
}