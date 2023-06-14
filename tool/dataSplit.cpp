#include "../melody2chord.h"
#include "../filereader.h"
int main() {
    mgnr::melody2chord::chordMap chordmap;
    QStringList ori_name;
    for (auto music : midiSearch::musicReader_3colume("aige.name.txt")) {
        ori_name.append(std::get<0>(music).name.c_str());
    }
    printf("%d\n",ori_name.size());
    int line = 0;
    for (auto music : midiSearch::musicReader_3colume("aige.txt")) {
        QString name = std::get<0>(music).name.c_str();
        auto nameList = name.split(" ");
        auto note_chord_count = nameList[nameList.size()-1].split("/");
        auto note_count = note_chord_count[0].split("-");
        auto chord_count = note_chord_count[1].split("-");
        if(note_count.size()!=chord_count.size()){
            std::cout << std::get<0>(music).name <<std::endl;
            continue;
        }
        size_t len = note_count.size();
        midiSearch::chord_t chord;
        midiSearch::melody_t melody;
        auto o_note = std::get<0>(music).melody;
        auto o_chord = std::get<0>(music).chord;
        int i_note  = 0;
        int i_chord = 0;
        for(auto i=0;i<len;++i){
            auto n_note = atoi(note_count[i].toStdString().c_str());
            auto n_chord = atoi(chord_count[i].toStdString().c_str());
            for(int j=0;j<n_note;++j){
                melody.push_back(o_note[i_note++]);
            }
            for(int j=0;j<n_chord;++j){
                chord.push_back(o_chord[i_chord++]);
            }

            QStringList note_rebuild_arr;
            for(auto it:melody){
                note_rebuild_arr.append(QString::number(it));
            }
            auto note_rebuild = note_rebuild_arr.join(",");
            QStringList chord_rebuild_arr;
            for(auto it:chord){
                QStringList ch_arr;
                for(auto note:it){
                    ch_arr.append(note==0?"0":QString::number(note));
                }
                chord_rebuild_arr.append(QString("[")+ch_arr.join(",")+"]");
            }

            auto chord_rebuild = chord_rebuild_arr.join(",");
            auto line_rebuild = ori_name.at(line++)+"|["+note_rebuild+"]|["+chord_rebuild+"]";
            printf("%s\n",line_rebuild.toStdString().c_str());

            melody.clear();
            chord.clear();
        }
    }
    return 0;
}