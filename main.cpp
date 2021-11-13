#include "midiMap.h"
#include "midiLoader.h"
#include "sampler.h"
#include "Options.h"
int main(int argc, char** argv){
    if(argc<5){
        return 0;
    }
    std::string input = argv[1];
    std::string output = argv[2];
    
    int noteChannel = atoi(argv[3]);
    int chordChannel = atoi(argv[4]);
    
    std::cout<<"输入:"<<input<<std::endl;
    std::cout<<"输出:"<<output<<std::endl;
    std::cout<<"旋律音轨:"<<noteChannel<<std::endl;
    std::cout<<"和弦音轨:"<<chordChannel<<std::endl;

    char buf_noteChannel[64];
    char buf_chordChannel[64];
    snprintf(buf_noteChannel,sizeof(buf_noteChannel),".%d",noteChannel);
    snprintf(buf_chordChannel,sizeof(buf_chordChannel),".%d",chordChannel);

    mgnr::midiMap midiMap;
    mgnr::loadMidi(midiMap,input);
    midiMap.updateTimeMax();
    
    std::cout<<"调性概率:"<<std::endl;
    midiMap.getBaseTone();

    std::cout<<"调性:"<<midiMap.baseTone<<std::endl;

    int lenInBeat = (midiMap.noteTimeMax/midiMap.TPQ);
    std::vector<int> notes;
    std::vector<std::set<int> > chords;
    char channel_note[64];
    char channel_chord[64];
    for(int i=0;i<lenInBeat;i=i+4){
        chords.push_back(mgnr::sampleChord(midiMap,i,4,midiMap.baseTone,[&](mgnr::note * n){
            return (strstr(n->info.c_str(), buf_chordChannel) != NULL);
        }));
        for(int j=0;j<8;++j){
            notes.push_back(mgnr::sample(midiMap,i+j*0.5,0.5,midiMap.baseTone,[&](mgnr::note * n){
                return (strstr(n->info.c_str(), buf_noteChannel) != NULL);
            }));
        }
    }
    auto fp = fopen(output.c_str(),"a");
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