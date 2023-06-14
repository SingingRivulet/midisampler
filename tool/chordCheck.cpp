#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "../filereader.h"
#include "../melody2chord.h"
int main(int argc, char** argv) {
    if (argc < 3) {
        return 0;
    }
    mgnr::melody2chord::chordMap m;
    auto out = fopen(argv[2], "w");
    for (auto it : midiSearch::musicReader_3colume(argv[1])) {
        midiSearch::chord_t & chords = std::get<0>(it).chord;
        QStringList lineRebuild;
        for (auto chord : chords) {
            QStringList chordRebuildArray;
            for (auto note : chord) {
                chordRebuildArray.append(QString::number(note));
            }
            QString chordRebuild = "[";
            chordRebuild.append(chordRebuildArray.join(","));
            chordRebuild.append("]");
            if (mgnr::melody2chord::inChordMap(m, chord)) {
                chordRebuild.append("*");
            }
            lineRebuild.append(chordRebuild);
        }
        fprintf(out,"[%s]\n",lineRebuild.join(",").toStdString().c_str());
    }
    if (out) {
        fclose(out);
    }
}