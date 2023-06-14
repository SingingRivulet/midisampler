#include "../melody2chord.h"
int main(){
    mgnr::melody2chord::chordMap m;
    std::vector<int> notes = {40,46,48};
    mgnr::melody2chord::inChordMap(m,notes);
    return 0;
}