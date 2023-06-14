#ifndef MGNR_SEQ2ID
#define MGNR_SEQ2ID
#include "melody2chord.h"
#include "cJSON.h"
#include <set>
namespace mgnr::seq2id {

struct dict{
    std::map<std::vector<int> , std::tuple<int,std::set<std::string>>> seq_id;
    int index = 0;
};

template<typename T>
inline int getIdBySeq(T & self,const std::vector<int> & seq){
    auto it = self.seq_id.find(seq);
    if(it==self.seq_id.end()){
        int minLen = -1;
        int minLen_val = -1;
        for(auto & it:self.seq_id){
            int len = melody2chord::calcEditDist(seq,it.first);
            if(minLen_val==-1 || len<minLen){
                minLen = len;
                minLen_val = std::get<0>(it.second);
            }
        }
        self.seq_id[seq] = minLen_val;
        return minLen_val;
    }else{
        return std::get<0>(it->second);
    }
}

template<typename T>
inline void load(T & self,const std::string & dic_path){}

template<typename T>
inline void save(T & self,const std::string & dic_path){
    auto obj = cJSON_CreateObject();
    auto seq_id = cJSON_CreateArray();
    auto id_seq = cJSON_CreateArray();
    cJSON_AddItemToObject(obj,"seq_id",seq_id);

    //写入seq_id
    {
        for (auto& it : self.seq_id) {
            auto item = cJSON_CreateObject();
            cJSON_AddItemToArray(seq_id, item);
            cJSON_AddNumberToObject(item, "val", std::get<0>(it.second));
            auto notes = cJSON_CreateArray();
            cJSON_AddItemToObject(item, "key", notes);
            for (auto note : it.first) {
                cJSON_AddItemToArray(notes, cJSON_CreateNumber(note));
            }
            auto chords = cJSON_CreateArray();
            cJSON_AddItemToObject(item, "chords", chords);
            for (auto & chord : std::get<1>(it.second)) {
                cJSON_AddItemToArray(chords, cJSON_CreateString(chord.c_str()));
            }
        }
    }

    auto s = cJSON_Print(obj);
    if(s){
        auto fp = fopen(dic_path.c_str(),"w");
        if(fp){
            fputs(s,fp);
            fclose(fp);
        }
        free(s);
    }
    cJSON_Delete(obj);
}

template<typename T>
inline int add(T & self,const std::vector<int> & seq,const std::string & chord){
    auto it = self.seq_id.find(seq);
    if (it == self.seq_id.end()) {
        ++self.index;
        auto & p = self.seq_id[seq];
        std::get<0>(p) = self.index;
        std::get<1>(p).clear();
        std::get<1>(p).insert(chord);
        return self.index;
    }else{
        auto & p = it->second;
        std::get<1>(p).insert(chord);
        return std::get<0>(p);
    }
}

}
#endif
