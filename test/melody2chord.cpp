#define MGNR_DEBUG
#include "../melody2chord.h"
int main(){
    mgnr::chordMap m;
    std::vector<int> notes;
    notes.push_back(1);
    notes.push_back(5);
    notes.push_back(8);
    auto res = mgnr::noteSeq2Chord(m,notes);
    printf("chord:%d-%s\n",std::get<0>(res),std::get<1>(res).c_str());
    return 0;
}