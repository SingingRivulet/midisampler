#include "Options.h"
#include "midiLoader.h"
#include "midiMap.h"
#include "sampler.h"
int main(int argc, char** argv) {
    bool processError = false;
    if (argc < 5) {
        return 0;
    }
    std::string input = argv[2];
    std::string output_ori = std::string(argv[3]);
    float delta = atof(argv[4]);
    std::cout << "输入:" << input << std::endl;
    std::cout << "delta:" << delta << std::endl;

    mgnr::midiMap midiMap;
    mgnr::loadMidi(midiMap, input);
    midiMap.updateTimeMax();

    std::map<float, std::string> tones;

    printf("从midi读取调性\n");
    for (auto it : midiMap.metaContent) {
        auto tm = (float)it.first / midiMap.TPQ;
        printf("%f=%s\n", tm, std::get<0>(it.second).c_str());
        if (std::get<0>(it.second).size() > 4) {
            if ((strstr(std::get<0>(it.second).c_str(), argv[1]) != NULL)) {
                tones[-tm] = std::get<0>(it.second);
            }
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
            /*
            fprintf(fp, "[");
            for (int j = 0; j < 4; ++j) {
                if (!first) {
                    fprintf(fp, ",");
                }
                fprintf(fp, "%d",
                        mgnr::sample(midiMap, i + j * 0.25, 0.25, 0, [&](mgnr::note* n) {
                            return true;
                        }));
                first = false;
                ++numNote;
            }
            fprintf(fp, "]|");
            */
            auto upper = tones.lower_bound(-(i + delta));
            printf("%d->", i);
            if (upper == tones.end()) {
                printf("null\n");
                fprintf(fp, "\n");
            } else {
                printf("%s\n", upper->second.c_str());
                fprintf(fp, "%s\n", upper->second.c_str());
            }
        }

        fclose(fp);
    }

    return 0;
}
