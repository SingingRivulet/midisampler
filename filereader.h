#pragma once
#include <qt5/QtCore/QString>
#include <qt5/QtCore/QStringList>
#include "generator.h"
#include "search.h"
namespace midiSearch {

//逐行读取
inline generator<char*> lineReader(const std::string& path) {
    auto fp = fopen(path.c_str(), "r");
    music res;
    if (fp) {
        auto buf = new char[65536];

        while (!feof(fp)) {
            bzero(buf, 65536);
            fgets(buf, 65536, fp);
            if (strlen(buf) > 1) {
                //切换协程
                co_yield buf;
            }
        }

        delete[] buf;
        fclose(fp);
    }
}

inline void str2melody(const QString& ostr, melody_t& melody) {
    QString str = ostr;
    auto notes_arr = str.replace("[", "").replace("]", "").split(",");
    for (auto it : notes_arr) {
        int note = it.toInt();
        melody.push_back(note);
    }
}

inline void str2chord(const QString& ostr, chord_t& chords) {
    QString str = ostr;
    auto chord_str_array = str.mid(2, str.size() - 4).remove("],").split("[");
    for (auto it : chord_str_array) {
        QStringList arr = it.split(",");
        std::vector<int> chord;
        for (auto n : arr) {
            int chord_note = n.toInt();
            if (chord_note > 0) {
                chord.push_back(chord_note);
            }
        }
        chords.push_back(std::move(chord));
    }
}

//3列的文件读取器（协程），返回值：music对象
inline generator<std::tuple<music,char*> > musicReader_3colume(const std::string& path) {
    music res;
    for (auto buf : lineReader(path)) {
        //开始切分字符串
        QString line = buf;
        auto line_array = line.simplified().split("|");
        auto name = line_array[0];
        auto notes_str = line_array[1];

        //旋律
        melody_t melody;
        str2melody(notes_str, melody);

        auto chord_str = line_array[2];
        //和弦
        chord_t chords;
        str2chord(chord_str, chords);

        res.name = name.toStdString();
        res.melody = melody;
        res.chord = chords;
        res.relativeMelody = buildRelativeArray(res.melody);
        //切换协程
        co_yield std::make_tuple(res,buf);
    }
}

//2列的文件读取器（协程），返回值：旋律
inline generator<melody_t> musicReader_2colume(const std::string& path) {
    music res;
    for (auto buf : lineReader(path)) {
        //开始切分字符串
        QString line = buf;
        auto line_array = line.simplified().split("|");
        auto notes_str = line_array[0];

        //旋律
        melody_t melody;
        str2melody(notes_str, melody);

        //切换协程
        co_yield melody;
    }
}

}  // namespace midiSearch