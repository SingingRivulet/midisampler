#include "../midiMap.h"
#include "../midiLoader.h"
#include "../sampler.h"
#include "../Options.h"
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
    
    printf("%s",res.c_str());
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
int main(int argc, char** argv){
    if(argc<3){
        return 0;
    }
    std::string input = argv[1];
    std::string output = argv[2];

    curl_global_init(CURL_GLOBAL_ALL);
    
    std::cout<<"输入:"<<input<<std::endl;
    std::cout<<"输出:"<<output<<std::endl;
    int noteChannel = 1;

    char buf_noteChannel[64];
    snprintf(buf_noteChannel,sizeof(buf_noteChannel),".%d",noteChannel);

    mgnr::midiMap midiMap;
    mgnr::loadMidi(midiMap,input);
    midiMap.updateTimeMax();
    
    int lenInBeat = (midiMap.noteTimeMax/midiMap.TPQ);
    std::vector<int> notes;
    char channel_note[64];
    std::vector<std::tuple<int,std::vector<int> > > resArr;
    int id = 0;
    for(int i=0;i<lenInBeat;i=i+1){
        for(int j=0;j<4;++j){
            notes.push_back(mgnr::sample(midiMap,i+j*0.25,0.25,0,[&](mgnr::note * n){
                return (strstr(n->info.c_str(), buf_noteChannel) != NULL);
            }));
            if(notes.size()>=255){
                getChord(notes,id*255,resArr);
                notes.clear();
                ++id;
            }
        }
    }
    if(!notes.empty()){
        getChord(notes,id*255,resArr);
        notes.clear();
        ++id;
    }
    printf("\n");
    //创建midi
    smf::MidiFile midifile;
    midifile.read(input);
    midifile.doTimeAnalysis();
    midifile.linkNotePairs();
    int track = midifile.addTrack();
    int TPQ=midifile.getTicksPerQuarterNote();
    int lastTime = 0;
    for(auto it:resArr){
        int tm = std::get<0>(it);
        printf("%d ",tm);
        printf(":");
        for(auto cit:std::get<1>(it)){
            printf("%d ",cit);
            midifile.addNoteOn(track,lastTime*TPQ/4,0,cit,90);
        }
        for(auto cit:std::get<1>(it)){
            midifile.addNoteOff(track,tm*TPQ/4,0,cit);
        }
        lastTime = tm;
        printf("\n");
    }
    midifile.write(output);
    return 0;
}