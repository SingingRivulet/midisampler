#include "../melody2chord.h"
#include "../seq2id.h"
#include "../filereader.h"
std::string buildChordStr(const std::vector<int>& arr,int baseChord,int  baseTone) {
    if(arr.empty()){
        return "null";
    }
    QStringList ch_arr;
    for (auto note : arr) {
        ch_arr.append(note<=0?"0":QString::number(
            mgnr::melody2chord::getToneLevelDelta(note+120,baseChord,baseTone)%14));
    }
    return ch_arr.join("-").toStdString();
}
int main() {
    mgnr::melody2chord::chordMap chordmap;
    auto fp = fopen("test.out.txt", "w");
    mgnr::seq2id::dict seqDict;
    for (auto music : midiSearch::musicReader_3colume("test.txt")) {
        auto m = std::get<0>(music);
        //printf("%s\n", m.name.c_str());
        std::vector<int>& notes = m.melody;
        auto chord_it = m.chord.begin();
        auto res = mgnr::melody2chord::getMusicSection(chordmap, notes, 4, 1.0, 0.5, 8);
        for (auto it : res) {
            //printf("%d_%s(w=%f) ", it.chord_base, it.chord_name.c_str(), it.weight);
            std::vector<int> noteGroup;
            for (auto& note : it.melody) {
                noteGroup.push_back(mgnr::melody2chord::getToneLevelDelta(note, it.chord_base, 0));
                if (noteGroup.size() >= 4) {
                    if (chord_it != m.chord.end()) {
                        fprintf(fp, "%d %s ", it.chord_base, it.chord_name.c_str());
                        for (auto& n : noteGroup) {
                            fprintf(fp, "%d ", n);
                        }
                        auto chord_val = buildChordStr(*chord_it,it.chord_base,0);
                        mgnr::seq2id::add(seqDict,noteGroup,chord_val);
                        fprintf(fp, "%s\n", chord_val.c_str());
                        chord_it++;
                    } else {
                        printf("%s\n", m.name.c_str());
                    }
                    noteGroup.clear();
                }
            }
            if (!noteGroup.empty()) {
                if (chord_it != m.chord.end()) {
                    fprintf(fp, "%d %s ", it.chord_base, it.chord_name.c_str());
                    for (auto& n : noteGroup) {
                        fprintf(fp, "%d ", n);
                    }
                    auto chord_val = buildChordStr(*chord_it, it.chord_base, 0);
                    mgnr::seq2id::add(seqDict, noteGroup, chord_val);
                    fprintf(fp, "%s\n", chord_val.c_str());
                    chord_it++;
                } else {
                    printf("%s\n", m.name.c_str());
                }
                noteGroup.clear();
            }
        }
        fprintf(fp, "\n");
        //printf("\n\n");
    }
    //mgnr::seq2id::save(seqDict,"test.json");
    fclose(fp);
    return 0;
}