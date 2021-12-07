#include "../midiMap.h"
#include "../midiLoader.h"
#include "../sampler.h"
#include <stdio.h>
#include <sstream>
inline void key2space(char & c){
    if(c=='\'' || c==',' || c=='[' || c==']'){
        c = ' ';
    }
}
int main(){
    auto file_note  = fopen("note.txt","r");
    auto file_chord = fopen("chord.txt","r");

    char buf_note[8192];
    char buf_chord[8192];
    char midi_name[256];

    int index = 0;
    if(file_note && file_chord){
        while((!feof(file_note)) && (!feof(file_chord))){
            bzero(buf_note  , sizeof(buf_note));
            bzero(buf_chord , sizeof(buf_chord));
            fgets(buf_note  , sizeof(buf_note)  , file_note);
            fgets(buf_chord , sizeof(buf_chord) , file_chord);

            for(int i=0;i<sizeof(buf_note);++i) key2space(buf_note[i]);
            for(int i=0;i<sizeof(buf_chord);++i)key2space(buf_chord[i]);

            std::istringstream iss_note(buf_note);
            std::istringstream iss_chord(buf_chord);
            snprintf(midi_name,sizeof(midi_name),"midi/%d.mid",++index);

            smf::MidiFile midifile;
            midifile.setTPQ(384);//0音轨
            midifile.addTrack();
            midifile.addTempo(0,0,40);
            midifile.addTrack();
            midifile.addTrack();

            int tm = 0;
            int lastNote = 0;
            int lastNoteStart = 0;
            while(iss_note.good()){
                std::string buf;
                iss_note>>buf;
                if(!buf.empty()){
                    int key = atoi(buf.c_str());
                    if(key!=lastNote){
                        if(lastNote!=0){
                            midifile.addNoteOn(1,lastNoteStart*24,0,lastNote,90);
                            midifile.addNoteOff(1,tm*24,0,lastNote);
                        }
                        lastNote = key;
                        lastNoteStart = tm;
                    }
                }else{
                    break;
                }
                ++tm;
            }
            if(lastNote!=0){
                midifile.addNoteOn(1,lastNoteStart*24,0,lastNote,90);
                midifile.addNoteOff(1,tm*24,0,lastNote);
            }

            tm = 0;
            std::string lastChord = "";
            int lastChordStart = 0;
            while(iss_chord.good()){
                std::string buf;
                iss_chord>>buf;
                if(!buf.empty()){
                    if(buf!=lastChord){
                        if(!lastChord.empty()){
                            #define processChord \
                            std::vector<int> notes; \
                            int len = lastChord.size()/2;\
                            char bufnote[3];\
                            bufnote[2] = '\0';\
                            for(int i=0;i<len;++i){\
                                bufnote[0] = lastChord.at(i*2);\
                                bufnote[1] = lastChord.at(i*2+1);\
                                int key = atoi(bufnote);\
                                notes.push_back(key);\
                            }\
                            for(auto it:notes){\
                                if(it!=0){\
                                    midifile.addNoteOn(2,lastChordStart*192,0,it,90);\
                                }\
                            }\
                            for(auto it:notes){\
                                if(it!=0){\
                                    midifile.addNoteOff(2,tm*192,0,it);\
                                }\
                            }
                            processChord;
                        }
                        lastChord = buf;
                        lastChordStart = tm;
                    }
                }else{
                    break;
                }
                ++tm;
            }
            if(!lastChord.empty()){
                processChord;
            }
            midifile.write(midi_name);
        }
    }

    if(file_note){
        fclose(file_note);
    }
    if(file_chord){
        fclose(file_chord);
    }

    return 0;
}