#include <memory.h>
#include <algorithm>
#include <iostream>
#include <qt5/QtCore/QString>
#include <qt5/QtCore/QStringList>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
char buf_line[65536];
int main(int argc, char** argv) {
    if (argc < 2) {
        return 0;
    }
    FILE* out = nullptr;
    if (argc >= 3) {
        out = fopen(argv[2], "w");
    }
    auto fp = fopen(argv[1], "r");
    int index = 0;
    if (fp) {
        QStringList song_melody, song_chord;
        QString lastTitle;
        while (!feof(fp)) {
            ++index;
            bzero(buf_line, sizeof(buf_line));
            fgets(buf_line, sizeof(buf_line), fp);
            int len = strlen(buf_line);
            if (len <= 1) {
                continue;
            }
            //处理文件
            QString line = buf_line;
            auto line_array = line.simplified().split("|");
            auto name = line_array[0];
            auto name_arr = name.split(" ");
            name_arr.pop_back();
            auto title = name_arr.join(" ");

            if (title != lastTitle) {
                if (!song_melody.empty()) {
                    QStringList melodyNum, chordNum;
                    for (auto it : song_melody) {
                        melodyNum.append(QString::number(it.split(",").size()));
                    }
                    for (auto it : song_chord) {
                        chordNum.append(QString::number(it.split("[").size() - 1));
                    }
                    auto rebuild = lastTitle + " " + melodyNum.join("-") + "/" + chordNum.join("-") + "|[" + song_melody.join(",") + "]|[" + song_chord.join(",") + "]";
                    fprintf(out, "%s\n", rebuild.toStdString().c_str());
                    song_melody.clear();
                    song_chord.clear();
                }
            }
            lastTitle = title;
            auto melody = line_array[1].replace(" ", "");
            auto chord = line_array[2].replace(" ", "");
            song_melody.append(melody.mid(1, melody.size() - 2));
            song_chord.append(chord.mid(1, chord.size() - 2));
        }
        if (!song_melody.empty()) {
            QStringList melodyNum, chordNum;
            for (auto it : song_melody) {
                melodyNum.append(QString::number(it.split(",").size()));
            }
            for (auto it : song_chord) {
                chordNum.append(QString::number(it.split("[").size() - 1));
            }
            auto rebuild = lastTitle + " " + melodyNum.join("-") + "/" + chordNum.join("-") + "|[" + song_melody.join(",") + "]|[" + song_chord.join(",") + "]";
            fprintf(out, "%s\n", rebuild.toStdString().c_str());
            song_melody.clear();
            song_chord.clear();
        }
        fclose(fp);
    }
    if (out) {
        fclose(out);
    }
    return 0;
}