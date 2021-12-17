#include "../melody2chord.h"
#include "../filereader.h"
int main() {
    mgnr::melody2chord::chordMap chordmap;
    for (auto music : midiSearch::musicReader_3colume("test.txt")) {
        auto m = std::get<0>(music);
        printf("%s\n",m.name.c_str());
        std::vector<int> & notes = m.melody;
        auto res = mgnr::melody2chord::getMusicSection(chordmap,notes , 4, 0.5, 1.0);
        for (auto it : res) {
            printf("%d_%s(w=%f) ", it.chord_base, it.chord_name.c_str(), it.weight);
            for (auto note : it.melody) {
                printf("%d ", note);
            }
            printf("\n");
        }
        printf("\n\n");
    }
    return 0;
}