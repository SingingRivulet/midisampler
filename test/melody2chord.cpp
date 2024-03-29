//#define MGNR_DEBUG
#include "../melody2chord.h"
int main(){
    mgnr::melody2chord::chordMap m;
    std::vector<int> notes = {
        45,45,45,45,45,45,45,45,0,0,0,0,0,0,0,0,0,0,0,0,45,45,45,45,50,50,50,50,50,50,50,50,50,50,46,46,46,46,46,46,0,0,0,0,0,0,0,0,0,0,0,0,43,43,43,43,48,48,48,48,48,48,50,50,48,48,48,48,48,48,48,48,0,0,0,0,0,0,0,0,0,0,48,48,48,48,48,48,48,48,48,48,50,50,45,45,45,45,45,45,45,45,45,45,0,0,0,0,0,0,0,0,45,45,45,45,45,45,45,45,0,0,0,0,0,0,0,0,0,0,45,45,45,45,45,45,50,50,50,50,50,50,50,50,50,50,46,46,46,46,46,46,0,0,0,0,0,0,0,0,0,0,0,0,43,43,43,43,48,48,48,48,48,48,50,48,48,48,48,48,48,48,48,48,0,0,0,0,0,0,0,0,0,0,0,0,48,48,48,48,48,48,48,48,48,48,48,50,50,50,50,50,50,50,50,50,0,0,0,0,0,0,0,0,0,0,0,0,50,50,50,50,53,53,53,53,53,53,50,50,53,53,53,53,53,53,53,53,53,53,53,53,53,53,53,53,0,0,0,0,50,50,50,50,53,53,53,53,53,53,50,50,52,52,52,52,52,52,52,52,0,0,0,0,0,0,0,0,0,0,0,0,52,52,52,52,52,52,52,52,52,52,52,52,53,53,53,53,53,53,50,50,50,50,50,50,48,48,46,46,46,46,46,46,46,46,46,46,53,53,46,46,46,46,43,43,45,45,45,45,45,45,45,45,0,0,0,0,0,0,0,0
    };
    auto res = mgnr::melody2chord::getMusicSection(m,notes,4,0.5,1.0);
    for(auto it:res){
        printf("%d_%s(w=%f) ",it.chord_base,it.chord_name.c_str(),it.weight);
        for(auto note:it.melody){
            printf("%d ",note);
        }
        printf("\n");
    }
    return 0;
}