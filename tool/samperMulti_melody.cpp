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
    std::cout << "输出:" << output_ori << std::endl;

    mgnr::midiMap midiMap;
    mgnr::loadMidi(midiMap, input);
    midiMap.updateTimeMax();

    std::map<float, std::string> tones;

    std::vector<std::pair<int, float>> melodyTracks;
    std::set<int> usedTrack;

    int chord_melody_mix = -1;

    printf("从midi读取调性\n");
    for (auto it : midiMap.metaContent) {
        auto tm = (float)it.first / midiMap.TPQ;
        printf("%f=%s\n", tm, std::get<0>(it.second).c_str());
        std::string lowstr = std::get<0>(it.second);
        std::transform(lowstr.begin(), lowstr.end(), lowstr.begin(), ::tolower);
        if (std::get<0>(it.second).size() > 1) {
            if (
                std::get<0>(it.second).at(0) >= 'A' &&
                std::get<0>(it.second).at(0) <= 'G' &&
                (strstr(std::get<0>(it.second).c_str(), "_") != NULL)) {
                tones[-tm] = std::get<0>(it.second);
                printf("和弦：%s 时间：%f\n", std::get<0>(it.second).c_str(), tm);
            } else if (
                ((strstr(std::get<0>(it.second).c_str(), "Track") != NULL) ||
                 (strstr(std::get<0>(it.second).c_str(), "track") != NULL)) &&
                (strstr(std::get<0>(it.second).c_str(), "melody") != NULL)) {
                if (strstr(std::get<0>(it.second).c_str(), "only") != NULL) {
                    auto melodyTrack = std::get<0>(it.second).at(5) - '0' + 1;
                    if (usedTrack.find(melodyTrack) == usedTrack.end()) {
                        melodyTracks.push_back(std::pair<int, float>(melodyTrack, 0));
                        usedTrack.insert(melodyTrack);
                    }
                } else {
                    printf("旋律和弦混合");
                    chord_melody_mix = std::get<0>(it.second).at(5) - '0' + 1;
                    break;
                }
            }  //else if (lowstr == "lead") {
               //  auto melodyTrack = std::get<1>(it.second);
               //  if (usedTrack.find(melodyTrack) == usedTrack.end()) {
               //      melodyTracks.push_back(std::pair<int, float>(melodyTrack, 0));
               //      usedTrack.insert(melodyTrack);
               //  }
            //}
        }
    }

    if (chord_melody_mix < 0) {
        if (melodyTracks.size() <= 0) {
            fprintf(stderr, "找不到旋律:%s\n", input.c_str());
            exit(0);
        }
        for (auto it : melodyTracks) {
            printf("旋律轨：%d\n", it);
        }

        //统计音轨音符数量
        for (auto& ch : melodyTracks) {
            for (auto it : midiMap.notes) {
                char buf_noteTrack[64];
                snprintf(buf_noteTrack, sizeof(buf_noteTrack), ".%d", ch.first);
                //std::cout << it->info << " " << buf_noteTrack;
                if (strstr(it->info.c_str(), buf_noteTrack) != NULL) {
                    ch.second += it->delay;
                    //std::cout << " " << ch.second << " 选中";
                }
                //std::cout << std::endl;
            }
        }
        std::sort(melodyTracks.begin(), melodyTracks.end(),
                  [](const std::pair<int, float>& A, const std::pair<int, float>& B) {
                      return A.second > B.second;
                  });
        for (auto& ch : melodyTracks) {
            printf("音轨：%d 时长：%f\n", ch.first, ch.second);
        }
    }

    auto fp = fopen(output_ori.c_str(), "w");
    if (fp) {
        int lenInBeat = (midiMap.noteTimeMax / midiMap.TPQ);
        std::vector<int> notes;
        int numNote = 0;
        int numChord = 0;
        for (int i = 0; i < lenInBeat; i = i + 1) {
            bool first = true;
            fprintf(fp, "[");
            for (int j = 0; j < 4; ++j) {
                if (!first) {
                    fprintf(fp, ",");
                }

                if (chord_melody_mix < 0) {
                    std::vector<int> melodyLevelNotes;
                    for (auto ch : melodyTracks) {
                        melodyLevelNotes.push_back(
                            mgnr::sample(midiMap, i + j * 0.25, 0.25, 0, [&](mgnr::note* n) {
                                char buf_noteTrack[64];
                                snprintf(buf_noteTrack, sizeof(buf_noteTrack), ".%d", ch.first);
                                return (strstr(n->info.c_str(), buf_noteTrack) != NULL);
                            }));
                    }
                    int melodyNote = 0;
                    for (auto melodyNote_it : melodyLevelNotes) {
                        if (melodyNote_it != 0) {
                            melodyNote = melodyNote_it;
                            break;
                        }
                    }
                    fprintf(fp, "%d", melodyNote);
                } else {
                    auto melodyNote = mgnr::sample(midiMap, i + j * 0.25, 0.25, 0, [&](mgnr::note* n) {
                        char buf_noteTrack[64];
                        snprintf(buf_noteTrack, sizeof(buf_noteTrack), ".%d", chord_melody_mix);
                        return (strstr(n->info.c_str(), buf_noteTrack) != NULL);
                    });
                    fprintf(fp, "%d", melodyNote);
                }
                first = false;
                ++numNote;
            }
            fprintf(fp, "]|");
            auto upper = tones.lower_bound(-i);
            printf("%d->", i);
            if (upper == tones.end()) {
                printf("null\n");
                fprintf(fp, "null\n");
            } else {
                printf("%s\n", upper->second.c_str());
                fprintf(fp, "%s\n", upper->second.c_str());
            }
        }

        fclose(fp);
    }

    return 0;
}
