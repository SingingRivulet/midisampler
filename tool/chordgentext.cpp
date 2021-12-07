#include "../midiMap.h"
#include "../midiLoader.h"
#include "../sampler.h"
#include <stdio.h>
#include <sstream>
#include <curl/curl.h>
size_t write_data(void *ptr, size_t size, size_t nmemb, void *res) {
    std::string data((const char*) ptr, (size_t) size * nmemb);
    *((std::string*) res) += data;
    return size * nmemb;
}
void getChord(const std::vector<int> & note,int beginId,std::vector<std::tuple<int,std::vector<int> > > & resArr){
    std::string url="http://127.0.0.1:8083/getChord?melody=";
    char buf[16];
    bool first = true;
    for(auto it:note){
        snprintf(buf,sizeof(buf),"%d",it);
        if(!first){
            url+=",";
        }
        url+=buf;
        first = false;
    }
    printf("url:%s\n",url.c_str());
    std::string res;
    CURL *curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &res);
    CURLcode rescode = curl_easy_perform(curl_handle);
    if (rescode != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(rescode));
    }
    
    printf("res:%s\n",res.c_str());
    curl_easy_cleanup(curl_handle);
    
    std::istringstream iss(res);
    std::string line;
    while(iss.good()){
        line.clear();
        std::getline(iss,line,'|');
        if(line.empty()){
            break;
        }
        auto len = line.size();
        for(int i=0;i<len;++i){
            if(line.at(i)==':' || line.at(i)=='[' || line.at(i)==',' || line.at(i)==']'){
                line.at(i) = ' ';
            }
        }
        std::istringstream liss(line);
        std::string str;
        std::tuple<int,std::vector<int> > t;
        liss>>str;
        std::get<0>(t) = atoi(str.c_str())+beginId;
        while(liss.good()){
            str.clear();
            liss>>str;
            if(str.empty()){
                break;
            }else{
                std::get<1>(t).push_back(atoi(str.c_str()));
            }
        }
        resArr.push_back(std::move(t));
    }
}
inline void key2space(char & c){
    if(c=='\'' || c==',' || c=='[' || c==']'){
        c = ' ';
    }
}

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

int main(){
    auto file_note  = fopen("note.txt","r");

    char buf_note[8192];
    char midi_name[256];

    int index = 0;
    std::vector<int> notes;
    std::vector<std::tuple<int,std::vector<int> > > resArr;
    if(file_note){
        while((!feof(file_note))){
            bzero(buf_note  , sizeof(buf_note));
            fgets(buf_note  , sizeof(buf_note)  , file_note);

            for(int i=0;i<sizeof(buf_note);++i) key2space(buf_note[i]);

            int basetone = getBaseTone(buf_note);
            printf("basetone:%d\n",basetone);

            std::istringstream iss_note(buf_note);
            snprintf(midi_name,sizeof(midi_name),"midi/%d.mid",++index);

            smf::MidiFile midifile;
            midifile.setTPQ(384);//0音轨
            midifile.addTrack();
            midifile.addTempo(0,0,80);
            midifile.addTrack();
            midifile.addTrack();

            int tm = 0;
            int lastNote = 0;
            int lastNoteStart = 0;
            int id = 0;
            int noteNum = 0;
            resArr.clear();
            notes.clear();
            while(iss_note.good()){
                std::string buf;
                iss_note>>buf;
                if(!buf.empty()){
                    int key = atoi(buf.c_str());
                    notes.push_back(key-basetone);
                    if(key!=lastNote){
                        if(lastNote!=0){
                            midifile.addNoteOn(1,lastNoteStart*48,0,lastNote,90);
                            midifile.addNoteOff(1,tm*48,0,lastNote);
                        }
                        lastNote = key;
                        lastNoteStart = tm;
                    }
                    if(notes.size()>=64){
                        getChord(notes,id*64,resArr);
                        notes.clear();
                        ++id;
                    }
                    ++noteNum;
                }else{
                    break;
                }
                ++tm;
            }

            if(!notes.empty()){
                getChord(notes,id*64,resArr);
                notes.clear();
                ++id;
            }
            if(lastNote!=0){
                midifile.addNoteOn(1,lastNoteStart*48,0,lastNote,90);
                midifile.addNoteOff(1,tm*48,0,lastNote);
            }
            int lastTime = 0;
            std::vector<int> lastChord;
            for(auto it:resArr){
                int tm = std::get<0>(it);
                auto & chord = std::get<1>(it);
                if(tm>noteNum){
                    break;
                }
                if(chord!=lastChord){
                    for(auto cit:lastChord){
                        midifile.addNoteOn(2,lastTime*384/8,0,cit+basetone,90);
                    }
                    for(auto cit:lastChord){
                        midifile.addNoteOff(2,tm*384/8,0,cit+basetone);
                    }
                    lastChord = chord;
                    lastTime = tm;
                }
            }
            if(!lastChord.empty()){
                for(auto cit:lastChord){
                    midifile.addNoteOn(2,lastTime*384/8,0,cit+basetone,90);
                }
                for(auto cit:lastChord){
                    midifile.addNoteOff(2,tm*384/8,0,cit+basetone);
                }
            }
            midifile.write(midi_name);
        }
    }

    if(file_note){
        fclose(file_note);
    }

    return 0;
}