#include "Options.h"
#include "midiLoader.h"
#include "midiMap.h"
#include "sampler.h"
int main(int argc, char** argv) {
    bool processError = false;
    if (argc < 3) {
        return 0;
    }
    std::string input = argv[1];
    std::string output_ori = std::string(argv[2]);

    std::cout << "输入:" << input << std::endl;

    mgnr::midiMap midiMap;
    mgnr::loadMidi(midiMap, input);
    midiMap.updateTimeMax();
    int lenInBeat = (midiMap.noteTimeMax / midiMap.TPQ);
    std::cout << "长度：" << lenInBeat << std::endl;
    //识别旋律轨道
    std::unordered_map<std::string, int> maxNoteCount;
    for (int i = 0; i < lenInBeat; i = i + 1) {
        for (int j = 0; j < 4; ++j) {
            int maxNote = -1;
            std::string maxNoteName;
            mgnr::fetch(midiMap, i + j * 0.25, 0.25, [&](auto n) {
                auto r = n->tone;
                if (r > maxNote) {
                    maxNote = r;
                    maxNoteName = n->info;
                }
            });
            if (!maxNoteName.empty()) {
                ++maxNoteCount[maxNoteName];
            }
        }
    }
    int maxNoteNum = -1;
    std::string noteTrack;
    for (auto& it : maxNoteCount) {
        if (it.second > maxNoteNum) {
            noteTrack = it.first;
        }
    }
    std::cout << "旋律音轨:" << noteTrack << std::endl;

    //识别节拍
    /*
    int beat_3 = 0;
    int beat_2 = 0;
    for (auto it : midiMap.notes) {
        if (strstr(it->info.c_str(), buf_chordChannel) != NULL) {
            int beatNum = round(it->delay / midiMap.TPQ);
            if (beatNum % 3 == 0) {
                ++beat_3;
            } else if (beatNum % 2 == 0) {
                ++beat_2;
            }
        }
    }
    if (beat_3 > beat_2) {
        printf("识别为三拍子，暂不支持处理\n");
        return 1;
    }
    */

    int basetone = 0;
    /*
    static const char* noteMap_1[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    static const char* noteMap_2[] = {"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"};
    static const char* noteMap_3[] = {"Am", "A#m", "Bm", "Cm", "C#m", "Dm", "D#m", "Em", "Fm", "F#m", "Gm", "G#m"};
    static const char* noteMap_4[] = {"Am", "Bbm", "Bm", "Cm", "Dbm", "Dm", "Ebm", "Em", "Fm", "Gbm", "Gm", "Abm"};
    if (argc >= 6) {
        for (int i = 0; i < 12; ++i) {
            if (
                strcmp(argv[5], noteMap_1[i]) == 0 ||
                strcmp(argv[5], noteMap_2[i]) == 0 ||
                strcmp(argv[5], noteMap_3[i]) == 0 ||
                strcmp(argv[5], noteMap_4[i]) == 0) {
                basetone = i;
                break;
            }
        }
        std::cout << "设置调性:";
    } else {
        std::cout << "识别调性:";
        basetone = midiMap.baseTone;
    }
    std::cout << basetone << "(" << noteMap_1[basetone] << ")" << std::endl;
    */

    std::vector<int> notes;
    std::vector<std::set<int> > chords;
    char channel_note[64];
    char channel_chord[64];
    int numNote = 0;
    int numChord = 0;
    for (int i = 0; i < lenInBeat; i = i + 1) {
        for (int j = 0; j < 4; ++j) {
            //chords.push_back(mgnr::sampleChord(midiMap, i + j * 0.0625, 0.0625, basetone, [&](mgnr::note* n) {
            //    return n->info != noteTrack;
            //}));
            //++numChord;
            notes.push_back(mgnr::sample(midiMap, i + j * 0.25, 0.25, basetone, [&](mgnr::note* n) {
                //return n->info == noteTrack;
                return (strstr(n->info.c_str(), ".0") != NULL);
            }));
            ++numNote;
        }
        chords.push_back(mgnr::sampleChord(midiMap, i, 1, basetone, [&](mgnr::note* n) {
            //return n->info != noteTrack;
            return (strstr(n->info.c_str(), ".1") != NULL);
        }));
        ++numChord;
    }
    //std::cout << "旋律:" << numNote << "和弦:" << numChord << std::endl;
    if (processError) {
        std::cout << "出现错误，停止写入" << std::endl;
        return 1;
    }
    std::cout << "旋律:" << notes.size() << "和弦:" << chords.size() << std::endl;
    auto fp = fopen(output_ori.c_str(), "a");
    if (fp) {
        fprintf(fp, "%s|[", input.c_str());

        bool first = true;
        for (auto it : notes) {
            if (first) {
                fprintf(fp, "%d", it);
            } else {
                fprintf(fp, ",%d", it);
            }
            first = false;
        }
        fprintf(fp, "]|[");
        first = true;
        for (auto it : chords) {
            if (!first) {
                fprintf(fp, ",");
            }
            {
                fprintf(fp, "[");
                bool first = true;
                for (auto n : it) {
                    if (first) {
                        fprintf(fp, "%d", n);
                    } else {
                        fprintf(fp, ",%d", n);
                    }
                    first = false;
                }
                fprintf(fp, "]");
            }
            first = false;
        }
        fprintf(fp, "]\n");
        fclose(fp);
    }
    return 0;
}
