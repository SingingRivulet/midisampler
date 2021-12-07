#include "midiMap.h"
#include "midiLoader.h"
#include "sampler.h"
#include "Options.h"
int main(int argc, char** argv){
    if(argc<5){
        return 0;
    }
    std::string input = argv[1];
    std::string output_ori = std::string(argv[2])+".ori.txt";
    std::string output_maj = std::string(argv[2])+".maj.txt";
    std::string output_min = std::string(argv[2])+".min.txt";
    
    int noteChannel = atoi(argv[3]);
    int chordChannel = atoi(argv[4]);
    
    std::cout<<"输入:"<<input<<std::endl;
    std::cout<<"旋律音轨:"<<noteChannel<<std::endl;
    std::cout<<"和弦音轨:"<<chordChannel<<std::endl;

    char buf_noteChannel[64];
    char buf_chordChannel[64];
    snprintf(buf_noteChannel,sizeof(buf_noteChannel),".%d",noteChannel);
    snprintf(buf_chordChannel,sizeof(buf_chordChannel),".%d",chordChannel);

    mgnr::midiMap midiMap;
    mgnr::loadMidi(midiMap,input);
    midiMap.updateTimeMax();
    //识别节拍
    int beat_3 = 0;
    int beat_2 = 0;
    for(auto it:midiMap.notes){
        if(strstr(it->info.c_str(), buf_chordChannel) != NULL){
            int beatNum = round(it->delay/midiMap.TPQ);
            if(beatNum%3==0){
                ++beat_3;
            }else if(beatNum%2==0){
                ++beat_2;
            }
        }
    }
    if(beat_3>beat_2){
        printf("识别为三拍子，暂不支持处理\n");
        return 1;
    }
    
    std::cout<<"调性概率:"<<std::endl;
    midiMap.getBaseTone();

    std::cout<<"调性:"<<midiMap.baseTone;
    if(!midiMap.isMajor){
        std::cout<<"（小调）";
    }
    std::cout<<std::endl;

    int lenInBeat = (midiMap.noteTimeMax/midiMap.TPQ);
    std::vector<int> notes;
    std::vector<std::set<int> > chords;
    char channel_note[64];
    char channel_chord[64];
    for(int i=0;i<lenInBeat;i=i+1){
        chords.push_back(mgnr::sampleChord(midiMap,i,1,midiMap.baseTone,[&](mgnr::note * n){
            return (strstr(n->info.c_str(), buf_chordChannel) != NULL);
        }));
        for(int j=0;j<4;++j){
            notes.push_back(mgnr::sample(midiMap,i+j*0.25,0.25,midiMap.baseTone,[&](mgnr::note * n){
                return (strstr(n->info.c_str(), buf_noteChannel) != NULL);
            }));
        }
    }
    auto fp = fopen(midiMap.isMajor?output_maj.c_str():output_min.c_str(),"a");
    if(fp){
        fprintf(fp,"[");
        bool first = true;
        for(auto it:notes){
            if(first){
                fprintf(fp,"%d",it);
            }else{
                fprintf(fp,",%d",it);
            }
            first = false;
        }
        fprintf(fp,"]|[");
        first = true;
        for(auto it:chords){
            if(!first){
                fprintf(fp,",");
            }
            {
                fprintf(fp,"[");
                bool first = true;
                for(auto n:it){
                    if(first){
                        fprintf(fp,"%d",n);
                    }else{
                        fprintf(fp,",%d",n);
                    }
                    first = false;
                }
                fprintf(fp,"]");
            }
            first = false;
        }
        fprintf(fp,"]\n");
        fclose(fp);
    }
    return 0;
}