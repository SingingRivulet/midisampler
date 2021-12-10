#include <sstream>
#include <iostream>
#include <algorithm>
#include <tuple>
#include <string>
#include <vector>
#include <memory.h>
#include <qt5/QtCore/QString>
#include <qt5/QtCore/QStringList>
inline bool isNotHalfNote(int note,int base){
    const static bool l[] = {true,false,true,false,true,true,false,true,false,true,false,true};
    return l[(note-base+12)%12];
}

inline bool checkMajor(int note){
    const static bool l[] = {true,false,false,false,true,false,false,true,false,false,false,false};
    return l[note%12];
}
const char * noteMap[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
const char * numberNoteMap[] = {"1","1#","2","2#","3","4","4#","5","5#","6","6#","7"};
int getBaseTone(const std::vector<int> & notes,bool chordMode = true){
    int single_note_count[12];//记录每个音符出现次数
    for(int i=0;i<12;++i){
        single_note_count[i] = 0;
    }
    int note_count = 0;

    for(auto note:notes){
        if(note!=0){
            note_count ++;
            single_note_count[((note)+12)%12] ++;
        }
    }

    if(note_count<=0){
        return 0;
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

    for(auto it:major_prob){
        printf("%3s:%2.2f ",noteMap[std::get<0>(it)],std::get<1>(it));
    }
    printf("\n");
    bool  haveEqualProb = false;
    int   equalProbNum  = 1;
    float equalProb     = std::get<1>(major_prob[0]);
    for(int i=1;i<12;++i){
        if(std::get<1>(major_prob[i])==equalProb){
            ++equalProbNum;
            haveEqualProb = true;
        }
    }
    if(haveEqualProb){
        printf("发现%d种可能\n",equalProbNum);
        //得到首位
        int note_begin=0,note_end=0;
        for(auto it=notes.begin();it!=notes.end();++it){
            if(*it!=0){
                note_begin = *it;
                break;
            }
        }
        //得到末位
        for(auto it=notes.rbegin();it!=notes.rend();++it){
            if(*it!=0){
                note_end = *it;
                break;
            }
        }
        //统计4和7
        std::vector<std::tuple<int,int,int,int,int,int> > relatuve_note_count;//{{调性,4个数,7个数,最大值,起始值,结束值},...}
        for(int i=0;i<equalProbNum;++i){
            int count_4 = single_note_count[(std::get<0>(major_prob[i])+5)%12]; //4
            int count_7 = single_note_count[(std::get<0>(major_prob[i])+11)%12];//7
            int tone = std::get<0>(major_prob[i]);
            relatuve_note_count.push_back(
                std::make_tuple(
                    tone,
                    count_4,
                    count_7,
                    std::max(count_4,count_7),
                    (note_begin-tone+12)%12,
                    (note_end-tone+12)%12
                )
            );
        }
        std::sort(relatuve_note_count.begin(), relatuve_note_count.end(),
            [&](const std::tuple<int,int,int,int,int,int> &x, std::tuple<int,int,int,int,int,int> &y) {
                if(chordMode){
                    int major_x = 0;
                    int major_y = 0;
                    if(std::get<4>(x)==0 || std::get<4>(x)==7 || std::get<4>(x)==9){
                        ++major_x;
                    }
                    if(std::get<5>(x)==0 || std::get<5>(x)==7 || std::get<5>(x)==9){
                        ++major_x;
                    }
                    if(std::get<4>(y)==0 || std::get<4>(y)==7 || std::get<4>(y)==9){
                        ++major_y;
                    }
                    if(std::get<5>(y)==0 || std::get<5>(y)==7 || std::get<5>(y)==9){
                        ++major_y;
                    }
                    if(major_x>major_y){
                        return true;
                    }
                    if(major_x<major_y){
                        return false;
                    }
                }
                return std::get<3>(x) > std::get<3>(y);
        });
        for(auto it:relatuve_note_count){
            printf(
                "假如为%s调，4个数:%d，7个数%d，开头：%s,结尾：%s\n",
                noteMap[std::get<0>(it)],
                std::get<1>(it),
                std::get<2>(it),
                numberNoteMap[std::get<4>(it)],
                numberNoteMap[std::get<5>(it)]);
        }
        baseTone = std::get<0>(relatuve_note_count.at(0));
    }else{
        baseTone = std::get<0>(major_prob[0]);
    }
    printf("最终确定调性：%d(%s)\n",baseTone,noteMap[baseTone%12]);
    return baseTone;
}
char buf_line[65526];
int main(int argc, char** argv){
    if(argc<2){
        return 0;
    }
    FILE * out = nullptr;
    if(argc>=3){
        out = fopen(argv[2],"w");
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
            //处理文件
            QString line = buf_line;
            auto line_array = line.simplified().split("|");
            auto name = line_array[0];
            auto notes_arr = line_array[1].replace("[","").replace("]","").split(",");
            std::vector<int> notes;
            for(auto it:notes_arr){
                int note = it.toInt();
                notes.push_back(note);
            }
            auto chord_str = line_array[2];
            auto chord_str_array = chord_str.mid(2,chord_str.size()-4).remove("],").split("[");

            //识别调性
            printf("%s\t行号：%d\n",name.toStdString().c_str(),index);
            
            //旋律
            printf("旋律部分\n");
            int tone = getBaseTone(notes);
            //重建hook格式
            QStringList note_rebuild_arr;
            for(auto it:notes){
                note_rebuild_arr.append(QString::number(it==0?it:it-tone));
            }
            auto note_rebuild = note_rebuild_arr.join(",");

            //和弦
            std::vector<std::vector<int> > chords;
            for(auto it:chord_str_array){
                QStringList arr = it.split(",");
                std::vector<int> chord;
                for(auto n:arr){
                    int chord_note = n.toInt();
                    if(chord_note>0){
                        chord.push_back(chord_note);
                    }
                }
                chords.push_back(std::move(chord));
            }
            //识别和弦调性
            std::vector<int> chord_melody;
            printf("和弦部分\n");
            for(auto it:chords){
                try{
                    int note = it.at(0);
                    chord_melody.push_back(note);
                }catch(...){}
            }
            tone = getBaseTone(chord_melody);
            //重建hook格式
            QStringList chord_rebuild_arr;
            for(auto it:chords){
                QStringList ch_arr;
                for(auto note:it){
                    ch_arr.append(note==0?"0":QString::number(note-tone));
                }
                chord_rebuild_arr.append(QString("[")+ch_arr.join(",")+"]");
            }

            auto chord_rebuild = chord_rebuild_arr.join(",");
            auto line_rebuild = name+"|["+note_rebuild+"]|["+chord_rebuild+"]";
            printf("\n");
            if(out){
                fprintf(out,"%s\n",line_rebuild.toStdString().c_str());
            }
        }
        fclose(fp);
    }
    if(out){
        fclose(out);
    }
    return 0;
}